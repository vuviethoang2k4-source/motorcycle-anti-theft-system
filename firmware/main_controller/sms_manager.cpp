#include "sms_manager.h"

#include <Arduino.h>

#include "config.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

namespace SmsManager {
namespace {

HardwareSerial gsmSerial(2);

enum class DriverState : uint8_t {
    BOOT_WAIT,
    INIT_SEND,
    INIT_WAIT,
    READY,
    READ_SMS_WAIT,
    DELETE_SMS_WAIT,
    SEND_SMS_WAIT_PROMPT,
    SEND_SMS_WAIT_RESULT,
    RETRY_WAIT
};

constexpr uint8_t INIT_COMMAND_COUNT = 5;
const char *const INIT_COMMANDS[INIT_COMMAND_COUNT] = {
    "AT",
    "ATE0",
    "AT+CMGF=1",
    "AT+CSCS=\"GSM\"",
    "AT+CNMI=2,1,0,0,0"
};

constexpr uint8_t SMS_QUEUE_SIZE = 4;
constexpr uint8_t REQUEST_QUEUE_SIZE = 4;

DriverState driverState = DriverState::BOOT_WAIT;

uint8_t initCommandIndex = 0;
uint32_t stateStartedAt = 0;
uint32_t operationDeadline = 0;

bool uartStarted = false;
bool ready = false;
bool operationOk = false;
bool operationError = false;
bool promptReceived = false;
bool cmgsReferenceReceived = false;

String lineBuffer;

int pendingSmsIndex = -1;
int activeSmsIndex = -1;
int deleteSmsIndex = -1;

String smsSender;
String smsBody;
bool waitingForSmsBody = false;

String smsQueue[SMS_QUEUE_SIZE];
uint8_t smsQueueHead = 0;
uint8_t smsQueueTail = 0;
uint8_t smsQueueCount = 0;
String activeOutgoingMessage;

SmsRequestType requestQueue[REQUEST_QUEUE_SIZE];
uint8_t requestQueueHead = 0;
uint8_t requestQueueTail = 0;
uint8_t requestQueueCount = 0;

uint32_t receivedSmsCount = 0;
uint32_t sentSmsCount = 0;
uint32_t failedSmsCount = 0;

void resetOperationFlags()
{
    operationOk = false;
    operationError = false;
    promptReceived = false;
    cmgsReferenceReceived = false;
}

void sendCommand(const String &command)
{
    gsmSerial.print(command);
    gsmSerial.print("\r");

    Serial.print("[GSM ->] ");
    Serial.println(command);
}

String canonicalPhoneNumber(const String &input)
{
    String digits;

    for (size_t index = 0; index < input.length(); ++index) {
        if (isDigit(input[index])) {
            digits += input[index];
        }
    }

    /*
     * Chuẩn hóa số Việt Nam:
     * +84xxxxxxxxx và 0xxxxxxxxx được đưa về cùng dạng 0...
     */
    if (digits.startsWith("84") &&
        digits.length() >= 10) {
        digits = "0" + digits.substring(2);
    }

    return digits;
}

bool isAuthorizedSender(const String &sender)
{
    return canonicalPhoneNumber(sender) ==
           canonicalPhoneNumber(
               String(OWNER_PHONE_NUMBER));
}

SmsRequestType parseRequest(String text)
{
    text.trim();
    text.toUpperCase();

    if (text == "ARM") {
        return SmsRequestType::ARM;
    }

    if (text == "DISARM") {
        return SmsRequestType::DISARM;
    }

    if (text == "FIND") {
        return SmsRequestType::FIND;
    }

    if (text == "SILENCE") {
        return SmsRequestType::SILENCE;
    }

    if (text == "STATUS") {
        return SmsRequestType::STATUS;
    }

    if (text == "LOCATION") {
        return SmsRequestType::LOCATION;
    }

    return SmsRequestType::NONE;
}

bool enqueueRequest(SmsRequestType request)
{
    if (request == SmsRequestType::NONE ||
        requestQueueCount >= REQUEST_QUEUE_SIZE) {
        return false;
    }

    requestQueue[requestQueueTail] = request;
    requestQueueTail =
        (requestQueueTail + 1) %
        REQUEST_QUEUE_SIZE;
    ++requestQueueCount;

    return true;
}

String extractPhoneFromCmgrHeader(
    const String &header)
{
    /*
     * Dạng thường gặp:
     * +CMGR: "REC UNREAD","+849...","","date"
     */
    const int firstComma =
        header.indexOf(',');

    if (firstComma < 0) {
        return "";
    }

    const int quoteStart =
        header.indexOf('"', firstComma);

    if (quoteStart < 0) {
        return "";
    }

    const int quoteEnd =
        header.indexOf('"', quoteStart + 1);

    if (quoteEnd < 0) {
        return "";
    }

    return header.substring(
        quoteStart + 1,
        quoteEnd);
}

int parseCmtiIndex(const String &line)
{
    const int commaPosition =
        line.lastIndexOf(',');

    if (commaPosition < 0) {
        return -1;
    }

    String indexText =
        line.substring(commaPosition + 1);
    indexText.trim();

    return indexText.toInt();
}

void handleCompletedIncomingSms()
{
    ++receivedSmsCount;

    Serial.print("[SMS] Sender: ");
    Serial.println(smsSender);

    Serial.print("[SMS] Body: ");
    Serial.println(smsBody);

    if (!isAuthorizedSender(smsSender)) {
        Serial.println(
            "[SMS] Ignored unauthorized sender.");
        return;
    }

    const SmsRequestType request =
        parseRequest(smsBody);

    if (request == SmsRequestType::NONE) {
        queueMessage(
            "INVALID COMMAND\n"
            "USE ARM DISARM FIND SILENCE STATUS LOCATION");
        return;
    }

    if (!enqueueRequest(request)) {
        queueMessage("SYSTEM BUSY");
    }
}

void processLine(String line)
{
    line.trim();

    if (line.length() == 0) {
        return;
    }

    Serial.print("[GSM <-] ");
    Serial.println(line);

    /*
     * URC báo có SMS mới có thể xuất hiện ở hầu hết trạng thái.
     */
    if (line.startsWith("+CMTI:")) {
        const int index = parseCmtiIndex(line);

        if (index >= 0 && pendingSmsIndex < 0) {
            pendingSmsIndex = index;
        }

        return;
    }

    switch (driverState) {
        case DriverState::INIT_WAIT:
        case DriverState::DELETE_SMS_WAIT:
            if (line == "OK") {
                operationOk = true;
            } else if (
                line == "ERROR" ||
                line.startsWith("+CME ERROR") ||
                line.startsWith("+CMS ERROR")) {
                operationError = true;
            }
            break;

        case DriverState::READ_SMS_WAIT:
            if (line.startsWith("+CMGR:")) {
                smsSender =
                    extractPhoneFromCmgrHeader(line);
                waitingForSmsBody = true;
                break;
            }

            if (line == "OK") {
                operationOk = true;
                break;
            }

            if (line == "ERROR" ||
                line.startsWith("+CME ERROR") ||
                line.startsWith("+CMS ERROR")) {
                operationError = true;
                break;
            }

            if (waitingForSmsBody &&
                smsBody.length() == 0) {
                smsBody = line;
                waitingForSmsBody = false;
            }
            break;

        case DriverState::SEND_SMS_WAIT_RESULT:
            if (line.startsWith("+CMGS:")) {
                cmgsReferenceReceived = true;
                break;
            }

            if (line == "OK" &&
                cmgsReferenceReceived) {
                operationOk = true;
                break;
            }

            if (line == "ERROR" ||
                line.startsWith("+CME ERROR") ||
                line.startsWith("+CMS ERROR")) {
                operationError = true;
            }
            break;

        default:
            break;
    }
}

void pollSerial()
{
    while (gsmSerial.available() > 0) {
        const char character =
            static_cast<char>(gsmSerial.read());

        /*
         * Dấu nhắc gửi nội dung SMS thường là '>'.
         * Nó có thể không kết thúc bằng ký tự xuống dòng.
         */
        if (driverState ==
                DriverState::SEND_SMS_WAIT_PROMPT &&
            character == '>') {
            promptReceived = true;
            continue;
        }

        if (character == '\r') {
            continue;
        }

        if (character == '\n') {
            if (lineBuffer.length() > 0) {
                processLine(lineBuffer);
                lineBuffer = "";
            }
            continue;
        }

        lineBuffer += character;

        if (lineBuffer.length() > 512) {
            lineBuffer.remove(0, 256);
        }
    }
}

void enterRetryWait()
{
    ready = false;
    driverState = DriverState::RETRY_WAIT;
    stateStartedAt = millis();

    Serial.println(
        "[GSM] Initialization will retry.");
}

void startReadSms(int index)
{
    activeSmsIndex = index;
    pendingSmsIndex = -1;

    smsSender = "";
    smsBody = "";
    waitingForSmsBody = false;

    resetOperationFlags();

    sendCommand(
        "AT+CMGR=" + String(activeSmsIndex));

    driverState = DriverState::READ_SMS_WAIT;
    operationDeadline =
        millis() +
        MainConfig::Timing::GSM_AT_TIMEOUT_MS;
}

void startDeleteSms(int index)
{
    resetOperationFlags();

    sendCommand(
        "AT+CMGD=" + String(index));

    driverState = DriverState::DELETE_SMS_WAIT;
    operationDeadline =
        millis() +
        MainConfig::Timing::GSM_AT_TIMEOUT_MS;
}

void popOutgoingMessage()
{
    activeOutgoingMessage =
        smsQueue[smsQueueHead];

    smsQueue[smsQueueHead] = "";
    smsQueueHead =
        (smsQueueHead + 1) %
        SMS_QUEUE_SIZE;
    --smsQueueCount;
}

void startSendSms()
{
    popOutgoingMessage();
    resetOperationFlags();

    sendCommand(
        "AT+CMGS=\"" +
        String(OWNER_PHONE_NUMBER) +
        "\"");

    driverState =
        DriverState::SEND_SMS_WAIT_PROMPT;

    operationDeadline =
        millis() +
        MainConfig::Timing::GSM_AT_TIMEOUT_MS;
}

void handleBootWait()
{
    if (millis() - stateStartedAt <
        MainConfig::Timing::GSM_BOOT_WAIT_MS) {
        return;
    }

    initCommandIndex = 0;
    driverState = DriverState::INIT_SEND;
}

void handleInitSend()
{
    resetOperationFlags();

    sendCommand(
        INIT_COMMANDS[initCommandIndex]);

    driverState = DriverState::INIT_WAIT;
    operationDeadline =
        millis() +
        MainConfig::Timing::GSM_AT_TIMEOUT_MS;
}

void handleInitWait()
{
    if (operationOk) {
        ++initCommandIndex;

        if (initCommandIndex >=
            INIT_COMMAND_COUNT) {
            ready = true;
            driverState = DriverState::READY;

            Serial.println(
                "[GSM] A7680C SMS ready.");
        } else {
            driverState = DriverState::INIT_SEND;
        }

        return;
    }

    if (operationError ||
        static_cast<int32_t>(
            millis() - operationDeadline) >= 0) {
        enterRetryWait();
    }
}

void handleReady()
{
    if (pendingSmsIndex >= 0) {
        startReadSms(pendingSmsIndex);
        return;
    }

    if (deleteSmsIndex >= 0) {
        const int index = deleteSmsIndex;
        deleteSmsIndex = -1;
        startDeleteSms(index);
        return;
    }

    if (smsQueueCount > 0) {
        startSendSms();
    }
}

void handleReadSmsWait()
{
    if (operationOk) {
        handleCompletedIncomingSms();

        deleteSmsIndex = activeSmsIndex;
        activeSmsIndex = -1;

        driverState = DriverState::READY;
        return;
    }

    if (operationError ||
        static_cast<int32_t>(
            millis() - operationDeadline) >= 0) {
        Serial.println(
            "[SMS] Cannot read incoming message.");

        deleteSmsIndex = activeSmsIndex;
        activeSmsIndex = -1;

        driverState = DriverState::READY;
    }
}

void handleDeleteSmsWait()
{
    if (operationOk ||
        operationError ||
        static_cast<int32_t>(
            millis() - operationDeadline) >= 0) {
        driverState = DriverState::READY;
    }
}

void handleSendSmsWaitPrompt()
{
    if (promptReceived) {
        gsmSerial.print(activeOutgoingMessage);
        gsmSerial.write(0x1A);

        Serial.print("[SMS ->] ");
        Serial.println(activeOutgoingMessage);

        resetOperationFlags();

        driverState =
            DriverState::SEND_SMS_WAIT_RESULT;

        operationDeadline =
            millis() +
            MainConfig::Timing::GSM_SMS_SEND_TIMEOUT_MS;
        return;
    }

    if (operationError ||
        static_cast<int32_t>(
            millis() - operationDeadline) >= 0) {
        ++failedSmsCount;
        activeOutgoingMessage = "";

        Serial.println(
            "[SMS] No CMGS prompt.");

        driverState = DriverState::READY;
    }
}

void handleSendSmsWaitResult()
{
    if (operationOk) {
        ++sentSmsCount;
        activeOutgoingMessage = "";

        Serial.println("[SMS] Sent successfully.");

        driverState = DriverState::READY;
        return;
    }

    if (operationError ||
        static_cast<int32_t>(
            millis() - operationDeadline) >= 0) {
        ++failedSmsCount;
        activeOutgoingMessage = "";

        Serial.println("[SMS] Send failed.");

        driverState = DriverState::READY;
    }
}

void handleRetryWait()
{
    if (millis() - stateStartedAt <
        MainConfig::Timing::GSM_RETRY_INTERVAL_MS) {
        return;
    }

    initCommandIndex = 0;
    driverState = DriverState::INIT_SEND;
}

}  // namespace

bool begin()
{
    gsmSerial.begin(
        MainConfig::GSM_BAUD,
        SERIAL_8N1,
        MainConfig::Pin::GSM_RX,
        MainConfig::Pin::GSM_TX);

    uartStarted = true;
    ready = false;

    driverState = DriverState::BOOT_WAIT;
    stateStartedAt = millis();

    Serial.print("[GSM] UART2 started at ");
    Serial.print(MainConfig::GSM_BAUD);
    Serial.println(" baud.");

    Serial.print("[GSM] ESP32 RX pin: GPIO");
    Serial.println(MainConfig::Pin::GSM_RX);

    Serial.print("[GSM] ESP32 TX pin: GPIO");
    Serial.println(MainConfig::Pin::GSM_TX);

    return true;
}

void update()
{
    if (!uartStarted) {
        return;
    }

    pollSerial();

    switch (driverState) {
        case DriverState::BOOT_WAIT:
            handleBootWait();
            break;

        case DriverState::INIT_SEND:
            handleInitSend();
            break;

        case DriverState::INIT_WAIT:
            handleInitWait();
            break;

        case DriverState::READY:
            handleReady();
            break;

        case DriverState::READ_SMS_WAIT:
            handleReadSmsWait();
            break;

        case DriverState::DELETE_SMS_WAIT:
            handleDeleteSmsWait();
            break;

        case DriverState::SEND_SMS_WAIT_PROMPT:
            handleSendSmsWaitPrompt();
            break;

        case DriverState::SEND_SMS_WAIT_RESULT:
            handleSendSmsWaitResult();
            break;

        case DriverState::RETRY_WAIT:
            handleRetryWait();
            break;
    }
}

bool isReady()
{
    return ready;
}

bool isBusy()
{
    return driverState != DriverState::READY;
}

bool getPendingRequest(SmsRequestType &request)
{
    if (requestQueueCount == 0) {
        return false;
    }

    request = requestQueue[requestQueueHead];

    requestQueueHead =
        (requestQueueHead + 1) %
        REQUEST_QUEUE_SIZE;
    --requestQueueCount;

    return true;
}

bool queueMessage(const String &message)
{
    if (message.length() == 0 ||
        smsQueueCount >= SMS_QUEUE_SIZE) {
        return false;
    }

    smsQueue[smsQueueTail] = message;

    smsQueueTail =
        (smsQueueTail + 1) %
        SMS_QUEUE_SIZE;
    ++smsQueueCount;

    return true;
}

uint32_t getReceivedSmsCount()
{
    return receivedSmsCount;
}

uint32_t getSentSmsCount()
{
    return sentSmsCount;
}

uint32_t getFailedSmsCount()
{
    return failedSmsCount;
}

}  // namespace SmsManager

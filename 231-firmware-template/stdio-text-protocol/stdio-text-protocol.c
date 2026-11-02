#include "stdio-text-protocol.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "log.h"

static char line_buffer[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static int line_buffer_length = 0;
command_t cmd;


void stdio_text_protocol_init(void)
{
    stdio_init_all();
    line_buffer_length = 0;
}

const command_t *stdio_text_protocol_handle(void)
{
    // читаем символ без ожидания; нет символа — возвращаем NULL
    int symbol = getchar_timeout_us(0);
    
    if (symbol == PICO_ERROR_TIMEOUT) {
        return NULL;
    }
    // LOG_DBG("got symbol %d\n", symbol);
    // LOG_DBG("line_buffer_length %d\n", line_buffer_length);
    // LOG_DBG("line_buffer %s\n", line_buffer);
    if (symbol == '\r' || symbol == '\n') {
        if (line_buffer_length == 0) {
            return NULL;
        }
        line_buffer[line_buffer_length] = 0;
        
        cmd.name = line_buffer;
        cmd.argc = 0;
        cmd.argv[0] = NULL;
        cmd.truncated = false;
        for (int i = 0; i < line_buffer_length - 1; ++i) {
            if (line_buffer[i] == '\0') {
                LOG_DBG("argc: %d\n new will be %s", cmd.argc, &line_buffer[i+1]);
                if (cmd.argc == STDIO_TEXT_PROTOCOL_MAX_ARGS) {
                    LOG_DBG("truncated");
                    cmd.truncated = true;
                    break;
                }
                cmd.argv[cmd.argc] = &line_buffer[i+1];
                cmd.argc++;
            }
        }
        putchar('\n');
        LOG_DBG("got command: %s, argc: %d\n", cmd.name, cmd.argc);
        for (int i = 0; i < cmd.argc; ++i) {
            LOG_DBG("arg[%d]: %s\n", i, cmd.argv[i]);
        }
        line_buffer_length = 0;
        return &cmd;

    }
    if (symbol == 0x08 || symbol == 0x7f) {
        if (line_buffer_length == 0) {
            return NULL;
        }
        putchar(' ');
        putchar('\b');
        // putchar(' ');
        // putchar('\b');

        line_buffer[line_buffer_length] = 0;
        
    }
    if (line_buffer_length == STDIO_TEXT_PROTOCOL_LINE_SIZE - 2) {
        return NULL;
    }
    if (symbol == ' ') {
        symbol = 0;
        putchar(' ');
    }
    putchar(symbol);
    line_buffer[line_buffer_length] = symbol;
    ++line_buffer_length;
    // конец строки: пустую строку пропускаем, иначе переводим строку в терминале,
    // делим строку на слова и возвращаем команду

    // Backspace (0x08 или 0x7f): убираем последний символ и стираем его на экране "\b \b"

    // печатный символ: кладём в буфер и повторяем в терминал;
    // буфер полон — помечаем строку как переполненную

    return NULL;
}
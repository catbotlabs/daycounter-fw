/**
 * @file src/main.cpp
 * @brief Main routine
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */
#include <Arduino.h>
#include "uart.h"
#include <avr/cpufunc.h>
#include <microprompt.h>

/*
volatile bool hey = false;
ISR(PCINT0_vect) {
    hey = true;
}
*/


using mprompt::CmdContext;
using mprompt::Command;

void cmd_ls(CmdContext ctx) {
    ctx.prompt->list_commands();
}

void cmd_reset(CmdContext ctx) {
    ctx.stream->println("sorry, not implemented.");
}


Command commands[] = {
    Command("ls", &cmd_ls),
    Command("reset", &cmd_reset)
};

mprompt::Prompt prompt(&sw_uart, commands, sizeof(commands) / sizeof(commands[0]));

void setup() {
    //GIMSK |= (1 << PCIE0);  // enable PCINT0 interrupts
    sw_uart.begin();
    prompt.greet(true);
}

void loop() {
    //delay(100);
    //sw_uart.println(sw_uart.available());
    if (got_rx) {
        got_rx = false;
        prompt.tick();
      //  sw_uart.print("R");
    //    prompt.tick();
    }
//    prompt.tick();
/*    if (hey) {
        hey = false;
        sw_uart.println("hey!");
        _NOP(); _NOP(); _NOP();         _NOP(); _NOP(); _NOP(); 
        hey = false;
    }*/
}
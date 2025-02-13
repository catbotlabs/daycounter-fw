/**
 * @file include/commands.h
 * @brief Handles incoming characters from the UART link,
 * typically running development-purpose commands.
 * 
 * Part of the CatBotLabs K001 Day Counter firmware.
 * 
 * Copyright (c) 2025, Winston Malinowski
 * This source code is provided under the terms of the MIT License.
 * (See the LICENSE file)
 */

#ifndef INCLUDE_COMMANDS_H_
#define INCLUDE_COMMANDS_H_

void handle_uart_char(char ch);

#endif  // INCLUDE_COMMANDS_H_

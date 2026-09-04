/*
 *  ╭─────────────────────────────────────────────────────────────╮
 *  │  💬 git-ez : prompt.h                                       │
 *  │  "The Conversationalist : Asking the Human for Input"       │
 *  ╰─────────────────────────────────────────────────────────────╯
 */

#ifndef PROMPT_H
#define PROMPT_H

#include <stdbool.h>
#include <stddef.h>

/**
 * Ask the user for text, offering a sensible default if they just hit Enter.
 */
void prompt_text(const char *label, const char *default_val, char *buffer, size_t max_len);

/**
 * Ask a friendly Yes/No question without any fuss.
 */
bool prompt_confirm(const char *label, bool default_yes);

/**
 * Present a neat numbered menu and let the user pick their favorite option.
 */
size_t prompt_choice(const char *label, const char *options[], size_t option_count, size_t default_idx);

/**
 * Pause the action until the user presses Enter (good for dramatic pauses).
 */
void prompt_wait_key(const char *message);

#endif // PROMPT_H

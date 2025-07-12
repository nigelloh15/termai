// shell.h
#pragma once

#include <string>
#include <vector>
#include <mutex>

// Initialize and start the shell subprocess.
// Returns true on success, false otherwise.
bool start_shell();

// Shutdown the shell subprocess and clean up resources.
void stop_shell();

// Send input text (a command) to the shell.
void write_to_shell(const std::string& input);

// Get a snapshot of the shell output.
// Thread-safe: copies internal output buffer under mutex.
std::vector<std::string> get_shell_output();

// Clear the shell output buffer.
void clear_shell_output();

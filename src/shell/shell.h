// shell.h
#pragma once

#include <string>
#include <vector>

// Initialize and start the shell subprocess.
// Returns true on success, false otherwise.
bool start_shell();

// Shutdown the shell subprocess and clean up resources.
void stop_shell();

// Send input text (a command) to the shell.
void write_to_shell(const std::string& input);

std::string getResponse(const std::string& input);

// Get a snapshot of the shell output.
// Thread-safe: copies internal output buffer under mutex.
std::vector<std::string>* get_shell_output();

// Clear the shell output buffer.
void clear_shell_output();

// Initialize the embedded Python interpreter and load the Python script.
// Returns true on success.
bool init_python();

// Finalize the embedded Python interpreter.
void finalize_python();

#ifndef COMMON_H
#define COMMON_H

/// Reads argument with especific size from the response pipe.
/// @param fcli_resp Response fifo.
/// @param arg Argument to be read.
/// @param count Size of argument.
ssize_t read_all(int fcli_resp, void* arg, size_t count);

/// Writes argument with especific size into the request pipe.
/// @param fcli_req Request fifo.
/// @param arg Argument to be write.
/// @param count Size of argument.
void write_all(int fcli_req, void* arg, size_t count);

#endif /* COMMON_H */
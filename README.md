# RC4 (Arcfour) Stream Cipher Implementation in C

## Overview

This is a C implementation of the RC4 stream cipher, also known as Arcfour. RC4 is a symmetric key stream cipher that was widely used in various protocols such as SSL/TLS and WEP. This implementation provides a simple, self-contained library for encryption and decryption using the RC4 algorithm.

## Files

- **arcfour.h** - Header file containing type definitions, macros, and function prototypes
- **arcfour.c** - Implementation of the RC4 algorithm
- **example.c** - Example program demonstrating encryption and decryption

## Features

- **Pure C implementation** - No external dependencies
- **Secure initialization** - Includes keystream "whitewashing" to discard initial output bytes
- **Symmetric operations** - Encryption and decryption use the same function
- **Memory safe** - Proper memory allocation and deallocation

## API Reference

### Data Structures

```c
struct s_arcfour {
    int16 i, j, k;          // Internal state indices
    int8 s[256];            // Permutation array (S-box)
};
typedef struct s_arcfour Arcfour;
```

### Functions

#### `Arcfour *rc4init(int8 *key, int16 size)`
Initializes the RC4 state with the provided key.
- **Parameters:**
  - `key`: Pointer to the encryption key (byte array)
  - `size`: Length of the key in bytes
- **Returns:** Pointer to initialized Arcfour structure
- **Note:** Automatically performs whitewashing (discards first 500 million bytes) for security

#### `int8 *rc4encrypt(Arcfour *p, int8 *cleartext, int16 size)`
Encrypts or decrypts data using the RC4 keystream.
- **Parameters:**
  - `p`: Pointer to initialized Arcfour structure
  - `cleartext`: Data to encrypt/decrypt (byte array)
  - `size`: Length of data in bytes
- **Returns:** Pointer to encrypted/decrypted data (malloc'd)

#### `int8 rc4byte(Arcfour *p)`
Generates the next byte of the RC4 keystream (internal use).

#### `void rc4uninit(Arcfour *p)`
Frees the RC4 state structure.
- **Note:** Actually implemented as a macro: `#define rc4uninit(x) free(x)`

### Macros

- `rc4decrypt(x, y, z)` - Alias for `rc4encrypt` (same operation)
- `rc4whitewash(x, y)` - Discards initial keystream bytes (500 million iterations)

## Compilation Instructions

### Compile the library and example:

```bash
# Compile all files together
gcc -o rc4_example arcfour.c example.c -std=c99 -Wall

# Or compile as a shared library
gcc -fPIC -shared -o libarcfour.so arcfour.c
gcc -o example example.c -L. -larcfour
```

### Build with different optimization levels:

```bash
# With optimization
gcc -o rc4_example arcfour.c example.c -O2 -std=c99

# For debugging
gcc -o rc4_example arcfour.c example.c -g -std=c99
```

## Usage Example

The `example.c` file demonstrates basic usage:

```c
#include "arcfour.h"

int main() {
    Arcfour *rc4;
    char *key = "tomatoes";
    char *text = "Shall i compare thee to a summer's day?";
    
    // Initialize with key
    rc4 = rc4init((int8 *)key, strlen(key));
    
    // Encrypt
    int8 *encrypted = rc4encrypt(rc4, (int8*)text, strlen(text));
    
    // Re-initialize with same key (decryption requires same state)
    rc4uninit(rc4);
    rc4 = rc4init((int8 *)key, strlen(key));
    
    // Decrypt
    int8 *decrypted = rc4decrypt(rc4, encrypted, strlen(text));
    
    rc4uninit(rc4);
    return 0;
}
```

## Security Notes

1. **Key Strength**: RC4 is considered insecure for modern applications. It should not be used in new systems.
2. **Initial Byte Discard**: This implementation discards the first 500 million bytes of keystream output to avoid biases in early output bytes.
3. **Key Management**: The security of RC4 depends entirely on the secrecy of the key.
4. **Deprecation**: RC4 has been deprecated in most security protocols due to various attacks.

## Limitations

- **8-bit data types**: Uses `int8` (unsigned char) for all byte operations
- **Fixed whitewash**: Always discards 500 million bytes during initialization
- **No error checking**: Minimal error handling in the API

## Memory Management

- `rc4init()` allocates memory for the Arcfour structure
- `rc4encrypt()` allocates memory for the output buffer
- `rc4uninit()` frees the Arcfour structure
- **Important**: The caller is responsible for freeing encrypted/decrypted data buffers

## Portability

The code uses standard C libraries and should compile on most systems with a C99-compliant compiler. The `_GNU_SOURCE` definition in the header may need adjustment for non-GNU systems.

## License

This code appears to be provided without an explicit license. Users should assume all rights reserved by the author unless otherwise specified.

## Disclaimer

This implementation is for educational purposes only. RC4 is cryptographically broken and should not be used in production systems requiring security. Use modern, vetted cryptographic libraries like OpenSSL or libsodium for real applications.

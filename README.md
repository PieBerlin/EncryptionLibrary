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
# Detailed Line-by-Line Explanation of arcfour.c

## File: arcfour.c

### Line 1: `#include "arcfour.h"`
**Purpose:** Includes the header file that contains function prototypes, type definitions, and macros needed for this implementation file.

---

### Lines 3-8: Function Signature and Variable Declarations
```c
export Arcfour *rc4init(int8 *key,int16 size){
    int16 x;
    int8 temp1;
    int8 temp2;
    Arcfour *p;
    int32 n;
```
**Detailed Explanation:**
- `export Arcfour *rc4init(int8 *key,int16 size)`: 
  - `export` is a macro defined in arcfour.h as `__attribute__((visibility("default")))` - makes the symbol visible when compiled as a shared library
  - Returns: `Arcfour *` - pointer to the RC4 state structure
  - Parameters: 
    - `int8 *key`: Pointer to the key (array of bytes)
    - `int16 size`: Length of the key in bytes
- Local variables:
  - `int16 x`: Loop counter
  - `int8 temp1, temp2`: Temporary variables for swapping values
  - `Arcfour *p`: Pointer to the RC4 state structure being initialized
  - `int32 n`: Used in the whitewashing loop

---

### Lines 10-15: Memory Allocation and Basic Initialization
```c
    if(!(p=malloc(sizeof(struct s_arcfour))))
        {assert_perror(errno);}
    
    for (x=0;x<256;x++)
        p->s[x]=0;
    p->i=p->j=p->k=0;
    temp1=temp2=0;
```
**Detailed Explanation:**
- Line 10-11: Allocates memory for the Arcfour structure
  - `malloc(sizeof(struct s_arcfour))`: Allocates memory equal to size of the structure
  - `if(!(p=...))`: Checks if allocation failed (returns NULL)
  - `assert_perror(errno)`: If allocation fails, prints error and aborts program
- Line 13-14: Initializes the S-box array to zeros
  - `for (x=0;x<256;x++)`: Loops through all 256 elements of the S-box
  - `p->s[x]=0`: Sets each element to 0
- Line 15: Initializes state variables i, j, k to 0
- Line 16: Initializes temporary variables to 0

---

### Lines 18-24: S-box Initialization (Identity Permutation)
```c
    /*for i from 0 to 255
        S[i] := i
    endfor*/
    for(p->i=0;p->i <256;p->i++)
        p->s[p->i]=p->i;
```
**Detailed Explanation:**
- Lines 18-21: Comment describing the RC4 key-scheduling algorithm (KSA) step 1
- Lines 22-23: Sets up identity permutation in S-box
  - `for(p->i=0;p->i <256;p->i++)`: Uses p->i as loop counter (modifies state)
  - `p->s[p->i]=p->i`: Sets S[i] = i for all i from 0 to 255

---

### Lines 26-44: Key Scheduling Algorithm (KSA)
```c
    /*j := 0 //we've already done this
    for i from 0 to 255
        j := (j + S[i] + key[i mod keylength]) mod 256
        swap values of S[i] and S[j]
    endfor*/
    for(p->i=0;p->i <256;p->i++){
        temp1=p->i % size;
        temp2=p->j+p->s[p->i]+ key[temp1];
        p->j=temp2 % 256;

        temp1=p->s[p->i];
        temp2=p->s[p->j];
        p->s[p->i]=temp2;
        p->s[p->j]=temp1;
    }
```
**Detailed Explanation:**
- Lines 26-32: Comment explaining the KSA algorithm
- Lines 33-43: Implementation of KSA:
  - Line 33: Loop through all 256 elements
  - Line 34: `temp1=p->i % size`: Calculates key index (i mod keylength)
  - Line 35: `temp2=p->j+p->s[p->i]+ key[temp1]`: 
    - `p->j`: Current j value
    - `p->s[p->i]`: Current S[i] value
    - `key[temp1]`: Key byte at calculated index
    - Sums them all
  - Line 36: `p->j=temp2 % 256`: Updates j modulo 256
  - Lines 38-42: Swap S[i] and S[j]:
    - Line 38: `temp1=p->s[p->i]`: Save S[i] to temp1
    - Line 39: `temp2=p->s[p->j]`: Save S[j] to temp2
    - Line 40: `p->s[p->i]=temp2`: Set S[i] to old S[j]
    - Line 41: `p->s[p->j]=temp1`: Set S[j] to old S[i]

---

### Lines 45-48: Reset State and Whitewash
```c
    p->i=p->j=0;
    rc4whitewash(n,p);
    return p;
}
```
**Detailed Explanation:**
- Line 45: `p->i=p->j=0`: Resets i and j to 0 after KSA (standard RC4 initialization)
- Line 46: `rc4whitewash(n,p)`: 
  - Macro defined in arcfour.h: `#define rc4whitewash(x,y) for(x=0;x<(MS*1000000);x++) {(volatile int8)rc4byte(y);}`
  - Generates and discards first 500 million bytes (MS=500, 500*1,000,000)
  - `volatile` prevents compiler from optimizing away the loop
  - Security measure: Discards initial keystream bytes that may have biases
- Line 47: `return p`: Returns pointer to initialized RC4 state

---

### Lines 50-53: rc4byte Function Signature
```c
int8 rc4byte(Arcfour *p){
    int16 tmp1,tmp2;
```
**Detailed Explanation:**
- `int8 rc4byte(Arcfour *p)`: 
  - Returns: `int8` - single byte of keystream
  - Parameters: `Arcfour *p` - pointer to RC4 state
- Local variables:
  - `int16 tmp1, tmp2`: Temporary variables for calculations

---

### Lines 56-61: Update i and j
```c
    /*while GeneratingOutput: we dont need this loop for now.. it will be handled in the encryption
    i := (i + 1) mod 256*/
    p->i=(p->i+1)%256;

    /*j := (j + S[i]) mod 256*/
    p->j=(p->j+p->s[p->i]) %256;
```
**Detailed Explanation:**
- Lines 56-58: Comment and implementation of i update
  - `p->i=(p->i+1)%256`: Increments i modulo 256 (wraps around)
- Lines 60-61: Comment and implementation of j update
  - `p->j=(p->j+p->s[p->i]) %256`: 
    - Adds S[i] to j
    - Modulo 256 to keep within array bounds

---

### Lines 63-69: Swap S[i] and S[j]
```c
    //swap values of S[i] and S[j]
    tmp1=p->s[p->i];
    tmp2=p->s[p->j];

    p->s[p->i]=tmp2;
    p->s[p->j]=tmp1;
```
**Detailed Explanation:**
- Lines 63-68: Performs swap operation:
  - Line 64: Save S[i] to tmp1
  - Line 65: Save S[j] to tmp2
  - Line 67: Set S[i] to tmp2 (old S[j])
  - Line 68: Set S[j] to tmp1 (old S[i])
- This swapping creates the pseudorandom nature of the keystream

---

### Lines 71-79: Generate Output Byte
```c
    //t := (S[i] + S[j]) mod 256
    tmp1=(p->s[p->i] +p->s[p->j]) % 256;

    //K := S[t]
    p->k=p->s[tmp1];
    
    //output K
    return p->k;
}
```
**Detailed Explanation:**
- Line 71: Comment explaining t calculation
- Line 72: `tmp1=(p->s[p->i] +p->s[p->j]) % 256`:
  - Adds S[i] and S[j] after swap
  - Modulo 256 for array index
- Line 75: Comment for K assignment
- Line 76: `p->k=p->s[tmp1]`:
  - Uses tmp1 as index into S-box
  - Stores result in p->k (state variable)
- Line 78: Comment
- Line 79: `return p->k`: Returns the generated keystream byte

---

### Lines 81-85: rc4encrypt Function Signature
```c
export int8 *rc4encrypt(Arcfour *p,int8 *cleartext,int16 size){
    int8 *ciphertext;
    int16 x;
```
**Detailed Explanation:**
- `export int8 *rc4encrypt(Arcfour *p,int8 *cleartext,int16 size)`:
  - `export`: Makes function visible in shared library
  - Returns: `int8 *` - pointer to encrypted/decrypted data
  - Parameters:
    - `Arcfour *p`: RC4 state (must be initialized)
    - `int8 *cleartext`: Input data (plaintext for encryption, ciphertext for decryption)
    - `int16 size`: Length of input data in bytes
- Local variables:
  - `int8 *ciphertext`: Pointer to output buffer
  - `int16 x`: Loop counter

---

### Lines 86-91: Memory Allocation for Output
```c
    ciphertext=(int8 *)malloc(size+1);
    if(!ciphertext){
        assert_perror(errno);
    }
```
**Detailed Explanation:**
- Line 86: `ciphertext=(int8 *)malloc(size+1)`:
  - Allocates memory for output (same size as input + 1 byte)
  - `size+1`: Extra byte might be for null terminator, though not used
- Lines 87-89: Error checking
  - If malloc fails (returns NULL), calls `assert_perror(errno)`
  - Prints error message and aborts program

---

### Lines 93-97: Encryption/Decryption Loop
```c
    for(x=0;x<size;x++){
        ciphertext[x]=cleartext[x]^ rc4byte(p);
    }
    
    return ciphertext;
}
```
**Detailed Explanation:**
- Line 93: `for(x=0;x<size;x++)`: Loops through each byte of input
- Line 94: `ciphertext[x]=cleartext[x]^ rc4byte(p)`:
  - XOR operation between input byte and keystream byte
  - `rc4byte(p)`: Generates next keystream byte
  - XOR is reversible: same operation for encryption and decryption
- Line 96: `return ciphertext`: Returns pointer to output buffer

## Key Algorithmic Insights:

### RC4 Algorithm Summary:
1. **Key Scheduling (KSA):** 
   - Initializes 256-byte S-box with identity permutation
   - Shuffles S-box based on key using swapping

2. **Pseudorandom Generation (PRGA):**
   - Updates indices i and j
   - Swaps S[i] and S[j]
   - Outputs S[S[i] + S[j] mod 256]

3. **Encryption/Decryption:**
   - XOR plaintext/ciphertext with keystream bytes

### Important Implementation Details:

1. **Whitewashing:** Discards first 500 million keystream bytes to avoid biases
2. **State Management:** i, j are reset to 0 after KSA, then incremented during PRGA
3. **Symmetry:** Encryption and decryption use identical XOR operation
4. **Memory:** Caller must free both the Arcfour structure and ciphertext buffers

### Security Considerations in This Implementation:

1. **Key Size:** Limited to 65535 bytes (int16)
2. **Whitewashing:** Extensive (500M bytes) for security but impacts performance
3. **Byte Operations:** All operations on 8-bit values with modulo 256 arithmetic
4. **State Isolation:** Each encryption session requires fresh initialization with key

# Detailed Explanation of First Two Functions in arcfour.c

## Function 1: `export Arcfour *rc4init(int8 *key, int16 size)`

### **What This Function Does:**
This function **initializes the RC4 cipher state** (also called the "context" or "session") using a provided encryption key. It sets up the internal state (S-box) that will be used to generate the pseudorandom keystream for encryption/decryption.

### **Input Parameters:**

| Parameter | Type | Description |
|-----------|------|-------------|
| `key` | `int8 *` | Pointer to the encryption key (array of bytes) |
| `size` | `int16` | Length of the key in bytes (max 65,535) |

### **What Happens Inside (Step by Step):**

1. **Memory Allocation**: Allocates memory for the RC4 state structure
2. **Initialization**: Sets all S-box values to 0, resets counters (i, j, k) to 0
3. **Identity Permutation**: Fills S-box with values 0, 1, 2, ..., 255 (in order)
4. **Key Scheduling Algorithm (KSA)**: Shuffles the S-box based on the key
   - For each position i from 0 to 255:
     - Calculates: `j = (j + S[i] + key[i % keylength]) % 256`
     - Swaps S[i] and S[j]
5. **Reset Counters**: Sets i and j back to 0 after KSA
6. **Whitewashing**: Generates and discards 500 million keystream bytes
   - This is a security measure to avoid biases in early keystream bytes
7. **Returns**: Pointer to the initialized RC4 state

### **Output:**
- Returns: `Arcfour *` (pointer to initialized RC4 state structure)
- The structure contains:
  - `s[256]`: The shuffled S-box (permutation table)
  - `i`, `j`, `k`: Internal counters ready for keystream generation

### **Visual Example:**
If key = "ABC" (ASCII values: 65, 66, 67):
```
Before KSA: S-box = [0, 1, 2, 3, ..., 255]
After KSA:  S-box = [shuffled based on key "ABC"]
```

### **Important Notes:**
- **Security**: The whitewashing step is CRITICAL - early RC4 keystream bytes have statistical biases
- **Performance**: Whitewashing 500M bytes takes time (performance vs security tradeoff)
- **Stateful**: The returned pointer maintains internal state between encryptions

---

## Function 2: `int8 rc4byte(Arcfour *p)`

### **What This Function Does:**
This function **generates a single pseudorandom byte** of the RC4 keystream. It's the core of the RC4 algorithm and is called repeatedly to produce the keystream that gets XORed with data.

### **Input Parameters:**

| Parameter | Type | Description |
|-----------|------|-------------|
| `p` | `Arcfour *` | Pointer to initialized RC4 state (from rc4init) |

### **What Happens Inside (Step by Step):**

1. **Update i**: `i = (i + 1) % 256`
   - Increments i, wraps around at 255
2. **Update j**: `j = (j + S[i]) % 256`
   - Adds current S[i] to j, wraps around at 255
3. **Swap**: Exchanges S[i] and S[j] in the S-box
4. **Calculate index**: `t = (S[i] + S[j]) % 256`
5. **Output**: Returns S[t] as the keystream byte

### **Output:**
- Returns: `int8` (single byte, 0-255)
- This byte is ready to be XORed with plaintext/ciphertext
- **Side effect**: Modifies the internal state (i, j, and S-box)

### **Visual Example of One Call:**
```
Initial state:
  i = 0, j = 0
  S-box = [some permutation of 0-255]

Step 1: i = (0 + 1) % 256 = 1
Step 2: j = (0 + S[1]) % 256  (depends on S[1] value)
Step 3: Swap S[1] and S[j]
Step 4: t = (S[1] + S[j]) % 256
Step 5: Return S[t] as keystream byte
```

### **Mathematical Properties:**
- **Deterministic**: Same key → same keystream sequence
- **Periodic**: Very long period (~2¹⁶⁰) but eventually repeats
- **Pseudorandom**: Appears random but is deterministic

---

## **How These Functions Work Together:**

### **Setup Phase (rc4init):**
```c
// User provides key
char *key = "MySecretKey";
int key_len = strlen(key);

// Initialize RC4 state
Arcfour *rc4_state = rc4init((int8 *)key, key_len);
// Now rc4_state->s[] contains key-dependent permutation
```

### **Keystream Generation Phase (rc4byte):**
```c
// Each call produces one keystream byte
int8 keystream_byte1 = rc4byte(rc4_state);  // First byte
int8 keystream_byte2 = rc4byte(rc4_state);  // Second byte
int8 keystream_byte3 = rc4byte(rc4_state);  // Third byte
// ... and so on
```

### **Encryption/Decryption:**
```c
// To encrypt/decrypt data
for (int i = 0; i < data_length; i++) {
    int8 keystream = rc4byte(rc4_state);
    ciphertext[i] = plaintext[i] ^ keystream;  // Encrypt
    // OR
    plaintext[i] = ciphertext[i] ^ keystream;  // Decrypt
}
```

## **Key Concept: Statefulness**

**RC4 is a stateful cipher:** Each call to `rc4byte` modifies the internal state. This means:

1. **Sequential**: You must generate keystream bytes in order
2. **Non-repeatable**: Can't jump to position N without generating bytes 0..N-1
3. **Synchronization**: Encryption and decryption must be in sync (same starting state, same number of bytes generated)

## **Real-World Analogy:**

Think of `rc4init` like **shuffling a deck of 256 cards** based on a password:
- The deck starts in order (0 to 255)
- Your password determines how you shuffle it
- The whitewashing is like cutting the deck multiple times after shuffling

Think of `rc4byte` like **drawing cards from the shuffled deck**:
- Each draw produces one "random" card (keystream byte)
- After drawing, you swap two cards in the deck (changing future draws)
- The next draw will be different because the deck changed

## **Security Implications:**

1. **Never reuse states**: Same key + same position = same keystream
2. **Whitewashing is essential**: Early bytes have patterns that can leak key information
3. **Key entropy matters**: Weak keys produce predictable keystreams

## **Common Usage Pattern:**

```c
// ENCRYPTION SIDE
Arcfour *encryptor = rc4init(key, key_len);
int8 *ciphertext = rc4encrypt(encryptor, plaintext, data_len);
rc4uninit(encryptor);

// DECRYPTION SIDE (must use same key)
Arcfour *decryptor = rc4init(key, key_len);  // Same key = same initial state
int8 *decrypted = rc4decrypt(decryptor, ciphertext, data_len);
rc4uninit(decryptor);
```

The magic is that `rc4encrypt` calls `rc4byte` repeatedly internally, so you don't usually call `rc4byte` directly unless building custom encryption logic.

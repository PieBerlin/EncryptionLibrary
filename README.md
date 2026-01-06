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

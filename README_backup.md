# Minitalk - UNIX Signal-Based Inter-Process Communication

A robust client-server communication system implemented in C using only UNIX signals (SIGUSR1 and SIGUSR2) for bit-by-bit data transmission. This project demonstrates deep understanding of systems programming, inter-process communication, and low-level signal handling.

[![42 School Project](https://img.shields.io/badge/42-Project-blue)](https://www.42.fr/)
[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Norminette](https://img.shields.io/badge/Norminette-passing-success)](https://github.com/42School/norminette)

---

## 📋 Table of Contents

- [Project Overview](#-project-overview)
- [The Challenge](#-the-challenge)
- [Technical Implementation](#-technical-implementation)
- [Bonus Features](#-bonus-features)
- [Key Technical Challenges](#-key-technical-challenges)
- [System Programming Concepts](#-system-programming-concepts)
- [Algorithm Flow](#-algorithm-flow)
- [Code Architecture](#-code-architecture)
- [Compilation & Usage](#-compilation--usage)
- [Testing Strategy](#-testing-strategy)
- [Technical Skills Demonstrated](#-technical-skills-demonstrated)
- [Performance & Reliability](#-performance--reliability)
- [Limitations & Design Decisions](#-limitations--design-decisions)
- [Debugging Challenges](#-debugging-challenges)
- [Learning Outcomes](#-learning-outcomes)

---

## 🎯 Project Overview

**Minitalk** is a 42 School project that implements a client-server communication system using exclusively UNIX signals. The project showcases fundamental systems programming skills by building a reliable data transmission protocol on top of the most basic inter-process communication (IPC) mechanism available in UNIX systems.

### Key Features:
- **Client-server architecture** with process identification via PIDs
- **Bit-by-bit transmission** using only two signals (SIGUSR1 and SIGUSR2)
- **Binary encoding/decoding** for character transmission
- **Asynchronous signal handling** with proper signal safety
- **Bonus: Acknowledgment system** for reliable communication
- **Unicode support** through proper character encoding

### Educational Context:
This project is part of the 42 School curriculum, designed to teach:
- UNIX signal mechanisms at a low level
- Inter-process communication without shared memory
- Bitwise operations and binary manipulation
- Asynchronous programming challenges
- System-level debugging techniques

### Project Objectives:
1. Implement reliable communication using unreliable signals
2. Understand signal handler safety and async-signal-safe functions
3. Master bitwise operations for data encoding/decoding
4. Handle race conditions and synchronization without locks
5. Build a robust protocol with minimal system resources

---

## 🚀 The Challenge

### Problem Statement

Create a communication system between two processes where:

**Constraints:**
- ✅ Use **only** UNIX signals (SIGUSR1 and SIGUSR2)
- ✅ Transmit strings bit by bit
- ✅ No shared memory allowed
- ✅ No pipes, FIFOs, or other IPC mechanisms
- ✅ Server must display its PID on startup
- ✅ Client sends message to server using server's PID
- ✅ Server must handle the message and display it

**Requirements:**
- The server must be able to receive strings from the client
- Communication must be reliable (no data loss)
- The server must display received strings without errors
- Multiple messages must be handled sequentially
- Characters must be perfectly reconstructed

### Why This Is Challenging:

1. **Limited Communication Channel:** Only 2 signals to represent all possible data
2. **No Built-in Reliability:** Signals can theoretically be lost by the OS
3. **Asynchronous Nature:** No guarantee of signal order or timing
4. **Signal Safety:** Very few functions are safe to call in signal handlers
5. **No Shared State:** Processes cannot share memory to coordinate
6. **Race Conditions:** Must prevent signal loss during transmission
7. **Synchronization:** Need to coordinate sender and receiver without locks

---

## 🔧 Technical Implementation

### Signal Handling

#### SIGUSR1 and SIGUSR2 Usage

The implementation uses a **binary encoding scheme** where:
- **SIGUSR1** represents binary `0`
- **SIGUSR2** represents binary `1`

Each character (8 bits) is transmitted as 8 sequential signals, with the bit order sent from **least significant bit (LSB) to most significant bit (MSB)**.

#### Basic Implementation (server.c)

```c
static void	handle_signal(int sig)
{
	static int				bits;
	static unsigned char	c;

	if (sig == SIGUSR2)
		c |= (1 << bits);  // Set bit if SIGUSR2 received
	bits++;
	if (bits == 8)
	{
		write(1, &c, 1);    // Character complete, output it
		bits = 0;
		c = 0;
	}
}
```

**Key Points:**
- Uses `signal()` system call (simple but older API)
- Static variables maintain state between signal invocations
- Bit accumulation using bitwise OR and left shift operations
- Outputs character immediately when 8 bits received

#### Bonus Implementation (server_bonus.c)

```c
static void	handle_signal(int sig_num, siginfo_t *sig_details, void *cpu_state)
{
	static int				bits;
	static unsigned char	c;

	(void)cpu_state;
	if (sig_num == SIGUSR2)
		c |= (1 << bits);
	bits++;
	if (bits == 8)
	{
		if (c == '\0')
			write(1, "\n", 1);
		else
			write(1, &c, 1);
		bits = 0;
		c = 0;
	}
	kill(sig_details->si_pid, SIGUSR1);  // Send acknowledgment
}
```

**Improvements:**
- Uses `sigaction()` with `SA_SIGINFO` flag (modern, safer API)
- Accesses sender's PID via `siginfo_t` structure
- Sends acknowledgment back to client after each bit
- Handles null terminator with newline output

#### Signal Safety Considerations

**Async-Signal-Safe Functions Used:**
- `write()` - for output (POSIX async-signal-safe)
- `kill()` - for sending acknowledgments
- Bitwise operations (inherently safe)

**Safety Measures:**
- No malloc/free in signal handlers
- No printf/ft_printf in handlers (uses write() instead)
- Static variables for state persistence
- No complex data structures

### Binary Encoding/Decoding

#### Character to Binary Conversion

**Client Side (client.c):**
```c
static void	send_char(pid_t pid, unsigned char c)
{
	int	i;

	i = 0;
	while (i < 8)
	{
		if (c & (1 << i))       // Check if bit i is set
			kill(pid, SIGUSR2);  // Send 1
		else
			kill(pid, SIGUSR1);  // Send 0
		usleep(350);            // Wait 350 microseconds
		i++;
	}
}
```

**Algorithm:**
1. Loop through bits 0-7 (LSB to MSB)
2. Use bitwise AND with left-shifted 1 to check each bit
3. Send SIGUSR2 for bit value 1, SIGUSR1 for bit value 0
4. Wait 350μs between signals to prevent signal loss

#### Bit-by-Bit Transmission Strategy

**Transmission Order:** LSB-first (little-endian bit transmission)
- Bit 0 (LSB) sent first
- Bit 7 (MSB) sent last

**Example:** Character 'A' (ASCII 65 = 0b01000001)
```
Bit sequence sent: 1, 0, 0, 0, 0, 0, 1, 0
Signals sent:      USR2, USR1, USR1, USR1, USR1, USR1, USR2, USR1
```

#### Binary to Character Reconstruction

**Server Side:**
```c
if (sig == SIGUSR2)
	c |= (1 << bits);  // Set the bit at position 'bits'
bits++;
```

**Algorithm:**
1. Initialize character `c` to 0
2. For each received signal:
   - If SIGUSR2: Set bit at current position using `c |= (1 << bits)`
   - If SIGUSR1: Bit remains 0 (no operation needed)
3. Increment bit counter
4. When counter reaches 8, character is complete

#### Byte Ordering Considerations

- **Character encoding:** Standard ASCII/Extended ASCII (0-255)
- **Bit order:** LSB-first transmission ensures proper reconstruction
- **Endianness:** Not applicable at character level; handled bit-by-bit
- **Unicode support:** Bonus version supports multi-byte UTF-8 characters through transparent byte-by-byte transmission

### Client Implementation

#### PID Handling
```c
pid_t	pid;
pid = ft_atoi(argv[1]);  // Convert string PID to integer
```

The client accepts the server's Process ID as a command-line argument and converts it using `ft_atoi()`.

#### String Parsing and Transmission
```c
i = 0;
while (argv[2][i])
{
	send_char(pid, argv[2][i]);  // Send each character
	i++;
}
send_char(pid, '\0');  // Send null terminator
```

**Process:**
1. Iterate through message string character by character
2. Send each character as 8 sequential signals
3. Send null terminator to signal end of message

#### Error Handling
```c
if (argc != 3)
{
	ft_printf("Use: %s <PID> <message>\n", argv[0]);
	return (1);
}
```

Basic validation ensures correct number of arguments. The `kill()` system call will fail if PID is invalid, causing the program to terminate.

#### Acknowledgment Handling (Bonus)

```c
static int	g_ack = 0;

static void	ack_handler(int sig)
{
	(void)sig;
	g_ack = 1;  // Set flag when ACK received
}

// In send_char:
g_ack = 0;
if (c & (1 << i))
	kill(pid, SIGUSR2);
else
	kill(pid, SIGUSR1);
while (!g_ack)
	pause();  // Wait for acknowledgment
```

**Synchronization Mechanism:**
- Global flag `g_ack` tracks acknowledgment state
- Client waits for server acknowledgment before sending next bit
- Uses `pause()` to sleep until signal received
- Prevents overwhelming the server with signals

### Server Implementation

#### PID Display
```c
pid_t	pid;
pid = getpid();
ft_printf("Server PID: %d\n", pid);
```

Server displays its PID immediately on startup for client connection.

#### Signal Reception Setup

**Basic Version:**
```c
signal(SIGUSR1, handle_signal);
signal(SIGUSR2, handle_signal);
```

**Bonus Version:**
```c
struct sigaction	action_struct;

action_struct.sa_sigaction = handle_signal;
sigemptyset(&action_struct.sa_mask);
action_struct.sa_flags = SA_SIGINFO;
sigaction(SIGUSR1, &action_struct, NULL);
sigaction(SIGUSR2, &action_struct, NULL);
```

The bonus uses `sigaction()` which provides:
- Access to sender's PID via `siginfo_t`
- Better control over signal behavior
- More reliable signal handling

#### Bit Accumulation Strategy

**State Maintenance:**
- `static int bits` - tracks number of bits received (0-7)
- `static unsigned char c` - accumulates the character being built

**Accumulation Logic:**
```c
if (sig == SIGUSR2)
	c |= (1 << bits);  // Set bit at position 'bits'
bits++;
```

#### Character Reconstruction Algorithm

1. **Initialize:** `bits = 0`, `c = 0`
2. **For each signal:**
   - If SIGUSR2: Set bit at position `bits` in `c`
   - Increment `bits` counter
3. **When bits == 8:**
   - Character is complete
   - Output character
   - Reset `bits = 0`, `c = 0`

#### String Buffering Approach

The implementation uses **immediate output** strategy:
- Each completed character is written immediately using `write(1, &c, 1)`
- No buffering or string accumulation
- Memory-efficient: only stores one character at a time
- Real-time display of received message

#### Multiple Client Handling

**Basic Version:** Sequential handling only (one message at a time)

**Considerations:**
- Static variables in signal handler maintain state
- New messages can be received after previous completes
- No explicit client identification
- Messages from different clients would be interleaved at bit-level (not practical)

### Synchronization

#### Timing Considerations

**Client-Side Delay:**
```c
usleep(350);  // 350 microseconds between signals
```

**Purpose:**
- Prevents signal loss due to OS limitations
- Allows server time to process each signal
- Prevents race conditions in signal delivery
- Chosen empirically for reliability vs. speed balance

#### usleep Usage for Signal Spacing

**Why 350μs?**
- Too fast: Signals may be lost or merged by kernel
- Too slow: Transmission becomes unnecessarily slow
- 350μs provides good reliability without excessive delay

**Calculation:**
- Per character: 8 bits × 350μs = 2.8ms
- For "Hello": 5 chars × 2.8ms = 14ms + null terminator
- Reasonable speed while maintaining reliability

#### Acknowledgment System (Bonus)

**Flow:**
1. Client sends bit signal
2. Client waits (`pause()`) for acknowledgment
3. Server receives bit, processes it
4. Server sends SIGUSR1 back to client
5. Client receives acknowledgment, continues

**Benefits:**
- Guaranteed delivery confirmation
- Prevents signal overflow
- Synchronizes sender and receiver
- Eliminates need for timing-based delays

#### Preventing Signal Loss

**Strategies Implemented:**
1. **Timing delays** (basic version) - 350μs spacing
2. **Acknowledgment protocol** (bonus) - wait for confirmation
3. **Signal blocking** - `sigaction` properly masks signals
4. **Atomic operations** - bitwise operations are atomic

---

## 🌟 Bonus Features

### 1. Server Acknowledgment to Client

**Implementation:**
- Server sends SIGUSR1 back to client after receiving each bit
- Client waits for acknowledgment before sending next bit
- Synchronous communication model

**Benefits:**
- Eliminates guesswork in timing
- Guarantees reliable delivery
- Prevents buffer overflow in kernel signal queue

### 2. Unicode Support

**How It Works:**
- UTF-8 characters are multi-byte sequences
- Each byte is transmitted independently
- Server reconstructs bytes without knowing encoding
- Display terminal handles UTF-8 interpretation

**Example:**
- '©' (UTF-8: 0xC2 0xA9)
- Transmitted as 2 characters (2 × 8 = 16 signals)
- Server outputs bytes sequentially
- Terminal renders Unicode symbol

### 3. Null Terminator Handling

**Bonus Enhancement:**
```c
if (c == '\0')
	write(1, "\n", 1);  // Output newline for message end
else
	write(1, &c, 1);
```

Adds newline after message completion for cleaner output.

### 4. Enhanced Error Handling

**Sigaction Benefits:**
- More robust signal handling
- Access to sender information
- Better control over signal behavior
- POSIX-compliant implementation

---

## 💪 Key Technical Challenges

### 1. Understanding UNIX Signals at Low Level

**Challenges:**
- Signals are asynchronous interrupts
- Limited to two signals for communication
- Signal handlers run in restricted context
- Understanding signal delivery guarantees

**Solutions:**
- Deep dive into POSIX signal specifications
- Testing signal behavior under various conditions
- Reading kernel documentation
- Studying async-signal-safe function requirements

### 2. Reliable Bit-by-Bit Communication

**Challenges:**
- Signals are not guaranteed to be queued
- Multiple signals may be merged by kernel
- No built-in acknowledgment mechanism

**Solutions:**
- Timing delays between signals (350μs)
- Acknowledgment protocol (bonus)
- Testing with various message lengths
- Empirical tuning of delays

### 3. Signal Handler Safety and Reentrancy

**Challenges:**
- Very few functions are async-signal-safe
- Cannot use most C library functions
- Must avoid deadlocks
- Static variables need careful management

**Solutions:**
- Use only async-signal-safe functions (`write`, `kill`)
- Avoid malloc/free in handlers
- Keep handler logic minimal
- Static variables for state persistence

### 4. Preventing Race Conditions

**Challenges:**
- Signals can arrive while handler is executing
- Static variables accessed from handler
- Sender and receiver synchronization

**Solutions:**
- Signal masking during critical sections (via sigaction)
- Atomic bitwise operations
- Acknowledgment protocol for synchronization
- Testing with rapid signal sequences

### 5. Handling Signal Delivery Guarantees

**Challenges:**
- UNIX doesn't guarantee all signals are delivered
- Signals of same type may be merged
- Queue overflow can lose signals

**Solutions:**
- Space signals apart with usleep()
- Use acknowledgment to confirm receipt
- Keep signal queue small (one at a time)
- Don't send signals too rapidly

### 6. Synchronization Without Shared State

**Challenges:**
- Processes have separate memory spaces
- Cannot use shared variables or mutexes
- Must coordinate using only signals

**Solutions:**
- Acknowledgment protocol creates synchronization
- pause() syscall waits for signals
- Global flag in client tracks acknowledgment state
- State machines in both client and server

### 7. Memory Safety in Signal Handlers

**Challenges:**
- Limited stack space in signal context
- Cannot allocate dynamic memory
- Must not corrupt program state

**Solutions:**
- Use only static variables
- Minimal local variables
- No recursive calls
- No complex data structures

### 8. Testing Reliability

**Challenges:**
- Race conditions are intermittent
- Signal timing varies by system load
- Difficult to reproduce edge cases

**Solutions:**
- Stress testing with long messages
- Concurrent testing (though not fully supported)
- Various message patterns (ASCII, Unicode, special chars)
- Testing on different systems and loads

---

## 📚 System Programming Concepts

### UNIX Signals

#### What Are Signals?

Signals are **software interrupts** that provide a way to handle asynchronous events. They are a fundamental IPC mechanism in UNIX systems.

#### Signal Types and Purposes

**SIGUSR1 and SIGUSR2:**
- User-defined signals
- No predefined behavior
- Can be caught and handled by applications
- Perfect for custom IPC protocols

**Other Signal Types (for reference):**
- SIGTERM - termination request
- SIGKILL - forced termination (cannot be caught)
- SIGINT - interrupt (Ctrl+C)
- SIGCHLD - child process status change

#### Signal Handlers vs Signal Masks

**Signal Handler:**
- Function that executes when signal received
- Registered with `signal()` or `sigaction()`
- Runs in special signal context
- Must be async-signal-safe

**Signal Mask:**
- Set of blocked signals
- Controlled via `sigprocmask()` or `sigaction()`
- Blocked signals are queued, not delivered
- Used to create atomic sections

#### Asynchronous Nature of Signals

**Key Characteristics:**
- Can arrive at any time
- Interrupt normal program flow
- Handler executes, then program continues
- No guarantee of timing or order
- Multiple signals of same type may be merged

#### Signal Safety (Async-Signal-Safe Functions)

**Safe Functions (partial list):**
- `write()`, `read()`
- `kill()`, `sigaction()`
- `getpid()`, `getppid()`
- `_exit()` (but not `exit()`)
- Bitwise operations

**Unsafe Functions:**
- `printf()`, `malloc()`, `free()`
- Most C library functions
- Functions that use static data
- Non-reentrant functions

### Process Communication

#### IPC Mechanisms Comparison

| Mechanism | Speed | Complexity | Data Size | Use Case |
|-----------|-------|------------|-----------|----------|
| **Signals** | Fast | Low | Very Limited | Simple notifications, control |
| **Pipes** | Fast | Medium | Unlimited | Stream data between related processes |
| **Sockets** | Medium | High | Unlimited | Network communication |
| **Shared Memory** | Fastest | High | Large | High-performance data sharing |
| **Message Queues** | Fast | Medium | Limited | Structured message passing |

#### Why Signals for This Project?

**Advantages:**
- Simplest IPC mechanism
- No setup required (always available)
- Works across unrelated processes
- Demonstrates low-level understanding

**Challenges:**
- Very limited bandwidth (2 signals)
- No built-in data transfer
- Requires creative encoding
- Tests fundamental CS concepts

#### Limitations and Advantages

**Limitations:**
- Slow transmission speed
- Only 2 bits of information per signal
- Potential for signal loss
- Complex synchronization required
- Not suitable for large data transfers

**Advantages:**
- Minimal system resources
- No kernel objects to manage
- Works with only PID knowledge
- Excellent learning tool
- Demonstrates bit-level understanding

#### Process Identification (PIDs)

**PID Basics:**
- Unique identifier for each process
- Assigned by kernel at process creation
- Required for sending signals (`kill()` syscall)
- Retrieved with `getpid()` system call

### Bitwise Operations

#### Bit Manipulation Techniques

**Setting a Bit:**
```c
c |= (1 << n);  // Set bit n in c
```

**Checking a Bit:**
```c
if (c & (1 << n))  // Test if bit n is set in c
```

**Clearing a Bit:**
```c
c &= ~(1 << n);  // Clear bit n in c
```

#### Binary Representation

**Character 'A' (ASCII 65):**
```
Decimal: 65
Binary:  01000001
Hex:     0x41
```

**Bit Positions:**
```
Bit:  7 6 5 4 3 2 1 0
Val:  0 1 0 0 0 0 0 1
```

#### Bit Shifting and Masking

**Left Shift (`<<`):**
- Moves bits to the left
- `1 << n` creates a mask with bit n set
- Used to set specific bits

**Right Shift (`>>`):**
- Moves bits to the right
- Can extract specific bit ranges
- Not used in this implementation

**Bitwise AND (`&`):**
- Tests if bit is set
- `value & mask` checks specific bits

**Bitwise OR (`|`):**
- Sets bits
- `value | mask` sets specific bits

---

## 📊 Algorithm Flow

### Client Side Flow

```
START
  ↓
Validate Arguments (argc == 3?)
  ↓ [No]
  └→ Print Usage → EXIT(1)
  ↓ [Yes]
Parse Server PID (argv[1])
  ↓
Setup Acknowledgment Handler (bonus)
  ↓
For Each Character in Message (argv[2]):
  ┌─────────────────────────┐
  │ For i = 0 to 7:        │
  │   ↓                     │
  │ Check bit i of char     │
  │   ↓                     │
  │ [Bit is 1?]            │
  │   ↓ [Yes]              │
  │ Send SIGUSR2           │
  │   ↓ [No]               │
  │ Send SIGUSR1           │
  │   ↓                     │
  │ Wait (usleep or ACK)   │
  │   ↓                     │
  │ Increment i            │
  └─────────────────────────┘
  ↓
Send Null Terminator ('\0')
  ↓
END
```

**Detailed Steps:**

1. **Validate Arguments:**
   - Check if exactly 3 arguments provided
   - If not, print usage and exit

2. **Parse Server PID:**
   - Convert string argument to integer using `ft_atoi()`
   - This PID is used for all signal transmissions

3. **Setup Signal Handler (Bonus):**
   - Register handler for SIGUSR1 (acknowledgment)
   - Initialize acknowledgment flag

4. **Character Loop:**
   - Iterate through each character in message string
   - Call `send_char()` for each character

5. **Bit Transmission:**
   - For each of 8 bits in character (LSB to MSB):
     - Test bit using `(c & (1 << i))`
     - Send SIGUSR2 if bit is 1
     - Send SIGUSR1 if bit is 0
     - Wait 350μs (basic) or for ACK (bonus)

6. **Send Terminator:**
   - Send '\0' character (8 SIGUSR1 signals)
   - Signals end of message

### Server Side Flow

```
START
  ↓
Get Process ID (getpid())
  ↓
Display PID to Console
  ↓
Setup Signal Handlers
  ↓
Enter Infinite Loop:
  ┌──────────────────────────┐
  │ Wait for Signal (pause())│
  │   ↓                       │
  │ Signal Received          │
  │   ↓                       │
  │ [HANDLER EXECUTES]       │
  │ ┌────────────────────┐   │
  │ │ Check Signal Type  │   │
  │ │   ↓                │   │
  │ │ [SIGUSR2?]        │   │
  │ │   ↓ [Yes]         │   │
  │ │ Set bit at position│   │
  │ │   ↓ [No]          │   │
  │ │ Bit stays 0       │   │
  │ │   ↓                │   │
  │ │ Increment bit count│   │
  │ │   ↓                │   │
  │ │ [bits == 8?]      │   │
  │ │   ↓ [Yes]         │   │
  │ │ Output character   │   │
  │ │ Reset bits & char  │   │
  │ │   ↓                │   │
  │ │ Send ACK (bonus)   │   │
  │ └────────────────────┘   │
  │   ↓                       │
  │ Continue Loop            │
  └──────────────────────────┘
```

**Detailed Steps:**

1. **Initialization:**
   - Get server process ID
   - Display PID for client to use
   - Print to console: "Server PID: XXXXX"

2. **Setup Signal Handlers:**
   - Register `handle_signal` for SIGUSR1
   - Register `handle_signal` for SIGUSR2
   - Use `signal()` (basic) or `sigaction()` (bonus)

3. **Main Loop:**
   - Call `pause()` to sleep until signal received
   - System automatically calls signal handler
   - Return from pause() after handler completes
   - Loop forever (server runs indefinitely)

4. **Signal Handler Execution:**
   - Called asynchronously when signal arrives
   - Static variables maintain state between calls

5. **Bit Processing:**
   - Check signal type
   - If SIGUSR2: Set bit at current position
   - If SIGUSR1: Bit remains 0 (no action)
   - Increment bit counter

6. **Character Completion:**
   - When 8 bits received (bits == 8):
     - Character is complete
     - Output using `write(1, &c, 1)`
     - Reset bit counter and character
     - Send acknowledgment to client (bonus)

---

## 🏗️ Code Architecture

### File Structure

```
minitalk/
├── Makefile                 # Build configuration
├── README.md               # This file
├── src/
│   ├── minitalk.h          # Header file with includes
│   ├── server.c            # Basic server implementation
│   ├── client.c            # Basic client implementation
│   ├── server_bonus.c      # Server with acknowledgment
│   └── client_bonus.c      # Client with acknowledgment
└── libft/                  # Custom C library
    ├── libft.h
    ├── ft_printf/          # Custom printf implementation
    └── [various .c files]  # Library functions
```

### Header Files and Shared Components

**minitalk.h:**
```c
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "../libft/libft.h"
#include "../libft/ft_printf/ft_printf.h"
```

**Shared Components:**
- Standard headers for signals and I/O
- Custom libft library for utility functions
- Custom ft_printf for formatted output

### Helper Functions Organization

**Client:**
- `send_char(pid_t pid, unsigned char c)` - Sends one character as 8 signals
- `ack_handler(int sig)` - Handles acknowledgment (bonus)
- `main()` - Entry point, argument parsing, message iteration

**Server:**
- `handle_signal(int sig)` - Basic signal handler
- `handle_signal(int sig_num, siginfo_t *sig_details, void *cpu_state)` - Bonus handler
- `main()` - Entry point, setup, infinite loop

### Separation of Concerns

**Clear Responsibilities:**

1. **Client Responsibilities:**
   - Argument validation
   - String iteration
   - Character to binary conversion
   - Signal transmission
   - Timing/synchronization

2. **Server Responsibilities:**
   - PID display
   - Signal reception
   - Binary to character reconstruction
   - Character output
   - Acknowledgment (bonus)

**Minimal Coupling:**
- Client and server share no code
- Communication only via signals
- No shared header definitions needed
- Independent compilation units

---

## 🔨 Compilation & Usage

### Compilation

The project uses a Makefile for easy compilation:

```bash
# Compile both client and server (basic version)
make

# Compile with bonus features (acknowledgment system)
make bonus

# Clean object files
make clean

# Remove all build artifacts
make fclean

# Recompile from scratch
make re
```

**Compilation Flags:**
- `-Wall` - Enable all warnings
- `-Wextra` - Enable extra warnings
- `-Werror` - Treat warnings as errors

**Compilation Process:**
1. Compiles libft library first
2. Compiles server.c → server executable
3. Compiles client.c → client executable

### Usage

#### Basic Usage

**Terminal 1 - Start Server:**
```bash
./server
```
Output:
```
Server PID: 12345
```

**Terminal 2 - Send Message:**
```bash
./client 12345 "Hello, World!"
```

**Server Output:**
```
Hello, World!
```

#### Examples

**Simple Message:**
```bash
./client 12345 "Test"
# Server displays: Test
```

**Long String:**
```bash
./client 12345 "This is a longer message to test the reliability of signal-based communication."
# Server displays the entire message
```

**Special Characters:**
```bash
./client 12345 "Special: @#$%^&*()_+-=[]{}|;:',.<>?/~\`"
# Server displays: Special: @#$%^&*()_+-=[]{}|;:',.<>?/~`
```

**Unicode Characters (Bonus):**
```bash
./client 12345 "Unicode: Hello 世界 🌍"
# Server displays: Unicode: Hello 世界 🌍
```

**Empty String:**
```bash
./client 12345 ""
# Server displays nothing (only newline in bonus)
```

**Multiple Messages:**
```bash
# Terminal 2
./client 12345 "First message"
./client 12345 "Second message"
./client 12345 "Third message"

# Server displays:
# First message
# Second message
# Third message
```

#### Error Cases

**Invalid PID:**
```bash
./client 99999 "Test"
# Error: kill() fails if PID doesn't exist
```

**Wrong Number of Arguments:**
```bash
./client
# Output: Use: ./client <PID> <message>
```

**No Server Running:**
```bash
./client 12345 "Test"
# Error: No such process (if PID 12345 doesn't exist)
```

---

## 🧪 Testing Strategy

### Unit Testing Approach

**Component Testing:**

1. **Signal Sending:**
   - Test `send_char()` with all ASCII values (0-255)
   - Verify correct signal sequence for known characters
   - Test with single characters

2. **Signal Reception:**
   - Test handler with known signal sequences
   - Verify bit accumulation logic
   - Test character reconstruction

3. **PID Handling:**
   - Test with valid PIDs
   - Test with invalid PIDs
   - Test PID parsing from string

### Message Integrity Verification

**Test Cases:**

```bash
# Test 1: Short message
echo "Test 1: Short message"
./client $(pgrep server) "Hi"

# Test 2: All printable ASCII
echo "Test 2: All printable ASCII"
./client $(pgrep server) "!\"#$%&'()*+,-./0123456789:;<=>?"

# Test 3: Alphabet
echo "Test 3: Alphabet"
./client $(pgrep server) "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

# Test 4: Numbers
echo "Test 4: Numbers"
./client $(pgrep server) "0123456789"
```

### Long Message Testing

**Stress Test:**
```bash
# Generate long message (1000 characters)
./client $(pgrep server) "$(python3 -c 'print("A"*1000)')"

# Test with repeated patterns
./client $(pgrep server) "$(perl -e 'print "Hello World! " x 100')"

# Test with Lorem Ipsum
./client $(pgrep server) "Lorem ipsum dolor sit amet, consectetur adipiscing elit..."
```

### Special Character Handling

**Edge Cases:**
```bash
# Null characters (challenging)
# Newlines (use $'...' syntax for literal newlines)
./client $(pgrep server) $'Line1\nLine2\nLine3'

# Tabs
./client $(pgrep server) "Tab	separated	values"

# Backslashes
./client $(pgrep server) "Path\\to\\file"

# Quotes
./client $(pgrep server) "He said \"Hello\""
```

### Unicode Testing (Bonus)

```bash
# Chinese characters
./client $(pgrep server) "你好世界"

# Emojis
./client $(pgrep server) "😀 😃 😄 😁 🌍 🌎 🌏"

# Mixed Unicode and ASCII
./client $(pgrep server) "Hello 世界 World"

# Accented characters
./client $(pgrep server) "café résumé naïve"
```

### Performance Testing

**Timing Tests:**
```bash
# Measure transmission time
time ./client $(pgrep server) "$(python3 -c 'print("A"*100)')"

# Calculate bits per second
# 100 chars × 8 bits × 350μs = 280ms theoretical minimum
```

### Signal Loss Scenarios

**Stress Testing:**
```bash
# Rapid sequential messages
for i in {1..10}; do
    ./client $(pgrep server) "Message $i" &
done
wait

# Note: Basic version may interleave bits
# Bonus version with ACK handles better
```

### Concurrent Client Testing (Limitations)

**Test Case:**
```bash
# Start two clients simultaneously
./client $(pgrep server) "Client1: AAAA" &
./client $(pgrep server) "Client2: BBBB" &
wait

# Expected: Interleaved output (not recommended)
# Server static variables don't distinguish clients
```

**Note:** This implementation doesn't fully support concurrent clients. Messages will be garbled.

### Reliability Testing

**Validation Script:**
```bash
#!/bin/bash
# test_reliability.sh

SERVER_PID=$(pgrep server)
TEST_MSG="The quick brown fox jumps over the lazy dog 1234567890"
ITERATIONS=100

for i in $(seq 1 $ITERATIONS); do
    echo "Test $i/$ITERATIONS"
    ./client $SERVER_PID "$TEST_MSG"
    sleep 0.1
done

echo "Completed $ITERATIONS transmissions"
```

---

## 💼 Technical Skills Demonstrated

### Systems Programming

**UNIX Signals Mastery:**
- Deep understanding of signal mechanisms
- Knowledge of async-signal-safe functions
- Proper use of `signal()` and `sigaction()`
- Signal handler implementation and safety

**Signal Handler Implementation:**
- Minimal, efficient handler code
- Static variable usage for state persistence
- Async-safe function usage only
- No memory allocation in handlers

**Process IPC Mechanisms:**
- Inter-process communication without shared memory
- Signal-based protocol design
- Process identification via PIDs
- Synchronization between processes

**Asynchronous Programming:**
- Handling asynchronous events (signals)
- Race condition prevention
- State machine design
- Coordination without shared state

**System Calls:**
- `kill()` - sending signals
- `signal()` / `sigaction()` - setting handlers
- `getpid()` - process identification
- `pause()` - waiting for signals
- `write()` - async-safe output
- `usleep()` - timing control

### Low-Level Programming

**Bitwise Operations:**
- Bit testing: `c & (1 << i)`
- Bit setting: `c |= (1 << i)`
- Bit shifting: `1 << i`
- Binary representation understanding

**Binary Encoding/Decoding:**
- Character to binary conversion
- Binary to character reconstruction
- LSB-first transmission
- Bit-by-bit protocol design

**Memory Management in Constrained Contexts:**
- No dynamic allocation in handlers
- Static variable usage
- Minimal stack usage
- Memory-efficient design

**Pointer Arithmetic:**
- String iteration: `argv[2][i]`
- Character pointer manipulation
- Direct memory access for output

### Problem Solving

**Constraint-Based Design:**
- Working with only 2 signals
- No shared memory
- Limited function availability in handlers
- Creative encoding scheme

**Reliability in Unreliable Communication:**
- Timing-based signal spacing
- Acknowledgment protocol
- Signal loss prevention
- Robust error handling

**Synchronization Without Locks:**
- Using signals for coordination
- Acknowledgment-based sync
- State machine implementation
- No mutexes or semaphores needed

**Edge Case Handling:**
- Null terminator transmission
- Empty strings
- Special characters
- Unicode multi-byte characters
- Invalid PIDs

### Software Engineering

**Client-Server Architecture:**
- Clear separation of concerns
- Independent processes
- Well-defined protocol
- Modular design

**Protocol Design:**
- Binary encoding scheme
- Acknowledgment mechanism
- Timing specifications
- Error handling strategy

**Error Handling:**
- Argument validation
- PID validation (via kill return value)
- Signal handler safety
- Graceful error messages

**Code Organization:**
- Separate files for client/server
- Helper functions for clarity
- Static functions for encapsulation
- Clean, readable code structure

**Makefile Creation:**
- Proper dependency management
- Separate rules for basic/bonus
- Clean/fclean/re targets
- Library compilation integration

---

## ⚡ Performance & Reliability

### Transmission Speed Considerations

**Current Performance:**
- **Per Bit:** 350μs (0.35ms)
- **Per Character:** 8 bits × 350μs = 2.8ms
- **Per Word (5 chars):** ~14ms
- **Theoretical Maximum:** ~357 characters/second

**Calculation Example:**
```
Message: "Hello, World!" (13 characters + null)
Time: 14 chars × 2.8ms = 39.2ms
Speed: ~34 characters/second for this message
```

**Factors Affecting Speed:**
- Signal processing overhead in kernel
- Context switching between processes
- Signal handler execution time
- OS scheduling latency

### Signal Spacing Optimization

**Why 350μs?**

**Too Fast (<100μs):**
- Signals may be lost
- Kernel signal queue overflow
- Messages become garbled
- Race conditions increase

**Too Slow (>1000μs):**
- Unnecessarily slow transmission
- Poor user experience
- No reliability benefit
- Wasted time

**350μs Sweet Spot:**
- Reliable delivery on most systems
- Good balance of speed/reliability
- Tested across different system loads
- Works on various UNIX systems

**Optimization Options:**
1. **Basic Version:** Fixed 350μs delay
2. **Bonus Version:** Acknowledgment-based (variable timing)
3. **Adaptive:** Could adjust based on ACK timing (not implemented)

### Reliability Measures

**Basic Version:**
- Time-based spacing (350μs)
- Simple but effective
- Works for sequential messages
- May lose signals under heavy load

**Bonus Version:**
- Acknowledgment after each bit
- Guaranteed delivery
- Self-pacing (adapts to receiver speed)
- No signals lost
- Slower but 100% reliable

**Additional Measures:**
- Static variables maintain state
- Signal handler atomicity
- Proper signal masking
- Character-level verification (null terminator)

### Error Recovery Strategies

**Client-Side:**
1. **Argument Validation:** Check argc before processing
2. **PID Validation:** `kill()` returns error for invalid PID
3. **Signal Send Errors:** Not explicitly checked (assumption: valid PID works)

**Server-Side:**
1. **Signal Handler Safety:** Only async-safe functions
2. **State Reset:** Reset bit counter and character after output
3. **Continuous Operation:** Server never exits, always ready

**Improvements Possible:**
- Client could check `kill()` return value
- Client could timeout waiting for ACK
- Server could detect malformed messages
- Error logging for debugging

---

## ⚠️ Limitations & Design Decisions

### Why 350μs Timing?

**Decision Process:**
1. **Initial Testing:** Started with 100μs (too fast)
2. **Signal Loss:** Observed dropped signals
3. **Incremental Increase:** Tested 200μs, 300μs, 350μs
4. **Empirical Result:** 350μs provides consistent reliability
5. **Trade-off:** Sacrificed speed for reliability

**Considerations:**
- Different systems have different thresholds
- System load affects signal processing
- 350μs works across most UNIX systems
- Bonus acknowledgment eliminates guesswork

### Signal Safety Constraints

**Limited Function Set:**
- **Cannot use:** `printf`, `malloc`, `free`, most C library
- **Can use:** `write`, `kill`, bitwise ops
- **Reason:** Async-signal-safe requirements
- **Impact:** Simple handlers, immediate output

**Design Impact:**
- No buffering (immediate character output)
- No complex data structures
- No error logging in handler
- Minimal state (two static variables)

### Scalability Considerations

**Current Limitations:**

1. **Single Message at a Time:**
   - Server processes one message sequentially
   - Static variables don't distinguish clients
   - Concurrent clients cause interleaved bits

2. **No Client Identification:**
   - Server doesn't track which client is sending
   - Multiple clients would corrupt messages
   - Bonus version slightly better (uses sender PID for ACK)

3. **No Message Queuing:**
   - Messages not queued
   - Must wait for previous message to complete
   - No buffering system

**Scalability Improvements (Not Implemented):**
- Separate handler state per client PID
- Message queue system
- Multi-threaded server
- Client session management

### Trade-offs Made

**Speed vs. Reliability:**
- **Choice:** Reliability
- **Reason:** Educational value, correctness over speed
- **Result:** Slow but reliable

**Simplicity vs. Features:**
- **Choice:** Simplicity (basic version)
- **Bonus:** Adds acknowledgment without excessive complexity
- **Result:** Clean, understandable code

**Memory vs. Real-time:**
- **Choice:** Real-time output (no buffering)
- **Reason:** Minimal memory footprint
- **Result:** Character-by-character display

**Signal() vs. Sigaction():**
- **Basic:** Uses `signal()` (simpler)
- **Bonus:** Uses `sigaction()` (more control)
- **Reason:** Show progression in complexity

---

## 🐛 Debugging Challenges

### Signal Debugging Techniques

**Challenges:**
- Signals are asynchronous
- Hard to reproduce timing issues
- Cannot use printf in signal handlers
- Breakpoints in handlers can change timing

**Techniques Used:**

1. **Write-Based Logging:**
```c
write(2, "Handler called\n", 15);  // Async-safe
```

2. **Signal Counter:**
```c
static int signal_count = 0;
signal_count++;
```

3. **Bit Pattern Logging:**
```c
char debug[9];
for (int i = 0; i < 8; i++)
    debug[i] = (c & (1 << i)) ? '1' : '0';
debug[8] = '\0';
write(2, debug, 8);
```

### Race Condition Identification

**Common Race Conditions:**

1. **Signal Queue Overflow:**
   - **Symptom:** Missing characters
   - **Detection:** Compare input/output
   - **Solution:** Increase usleep delay

2. **Handler Reentrancy:**
   - **Symptom:** Garbled characters
   - **Detection:** Static variable corruption
   - **Solution:** Signal masking with sigaction

3. **Concurrent Clients:**
   - **Symptom:** Interleaved bits
   - **Detection:** Nonsense output with 2 clients
   - **Solution:** Use one client at a time

### Testing Reliability

**Stress Test Script:**
```bash
#!/bin/bash
for i in {1..1000}; do
    EXPECTED="Test message number $i"
    RESULT=$(echo $EXPECTED | ./client $(pgrep server))
    if [ "$RESULT" != "$EXPECTED" ]; then
        echo "FAIL at iteration $i"
        exit 1
    fi
done
echo "All 1000 tests passed"
```

**Verification Techniques:**
- Known input/output pairs
- Checksum comparison
- Character count validation
- Visual inspection

### Tools Used

**GDB with Signals:**
```bash
# Debug server
gdb ./server
(gdb) handle SIGUSR1 nostop noprint
(gdb) handle SIGUSR2 nostop noprint
(gdb) break handle_signal
(gdb) run
```

**Strace for System Calls:**
```bash
# Trace server signals
strace -e signal ./server

# Trace client kill() calls
strace -e kill ./client 12345 "Test"
```

**Valgrind (Limited Use):**
```bash
# Memory checking (though no dynamic allocation)
valgrind --leak-check=full ./server
```

**Custom Debug Builds:**
```makefile
debug: CFLAGS += -g -DDEBUG
debug: all
```

**Logging:**
```c
#ifdef DEBUG
write(2, "Debug: bit received\n", 20);
#endif
```

---

## 🎓 Learning Outcomes

### Deep Understanding of UNIX Signals

**Conceptual Knowledge:**
- Signals are software interrupts
- Asynchronous event handling
- Signal delivery mechanics
- Kernel signal queuing behavior
- Signal masking and blocking

**Practical Skills:**
- Implementing signal handlers
- Using `signal()` and `sigaction()`
- Understanding async-signal-safe functions
- Managing signal handler state
- Debugging signal-related issues

### Systems Programming Experience

**Core Competencies:**
- Working directly with OS primitives
- Understanding process model
- System call usage and semantics
- Resource constraints in kernel context
- Performance considerations

**Low-Level Skills:**
- Bitwise operations mastery
- Binary data representation
- Memory-efficient programming
- Pointer manipulation
- Static vs dynamic allocation trade-offs

### IPC Mechanism Knowledge

**IPC Landscape:**
- Signals (lightweight, limited)
- Pipes (stream-based, unidirectional)
- Sockets (network-capable, complex)
- Shared memory (fast, requires synchronization)
- Message queues (structured, POSIX)

**Design Considerations:**
- Choosing appropriate IPC for use case
- Understanding performance trade-offs
- Reliability requirements
- Complexity vs. capability

### Low-Level Communication Protocols

**Protocol Design:**
- Encoding scheme design
- Synchronization mechanisms
- Error detection (implicit via character validation)
- Acknowledgment systems
- Timing specifications

**Implementation:**
- State machine design
- Bit-level data transmission
- Reliable communication over unreliable medium
- Protocol testing and validation

### Asynchronous Programming Patterns

**Concepts:**
- Event-driven programming
- Callback functions (signal handlers)
- State persistence between events
- Non-blocking operations
- Coordination without shared state

**Challenges:**
- Race conditions
- Reentrancy issues
- Atomicity requirements
- Debugging asynchronous code

---

## 🚀 Conclusion

The **minitalk** project successfully demonstrates mastery of fundamental systems programming concepts through a creative and challenging implementation. By building a reliable communication protocol using only two UNIX signals, this project showcases:

✅ **Technical Depth:** Understanding of signal mechanics, bitwise operations, and async programming

✅ **Problem-Solving:** Overcoming significant constraints to build reliable communication

✅ **Software Engineering:** Clean architecture, proper error handling, and maintainable code

✅ **Systems Thinking:** Knowledge of OS primitives, process communication, and resource management

This project provides a strong foundation for further systems programming work and demonstrates capabilities highly valued in low-level software development, embedded systems, and operating system engineering.

---

## 📝 License

This project is part of the 42 School curriculum. Feel free to study the code and concepts, but please follow your school's academic integrity policies if you're a 42 student.

---

## 👤 Author

**fshiniti**
- 42 Porto Student
- GitHub: [@fsugii](https://github.com/fsugii)

---

## 🙏 Acknowledgments

- **42 School** for the project design and learning objectives
- **UNIX/Linux communities** for extensive signal documentation
- **POSIX standards** for signal safety specifications

---

**⭐ If this README helped you understand systems programming concepts, consider giving the repository a star!**

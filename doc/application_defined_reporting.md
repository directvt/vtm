### Draft

# Application Defined Reporting (ADR) via Scriptable Sandboxing

## Overview

Application Defined Reporting (ADR) is a paradigm shift in terminal-to-application communication. Instead of relying on hardcoded keyboard or mouse protocols (like xterm, kitty, or win32-input-mode), ADR allows the application to dynamically define its own event-reporting logic. This is achieved by deploying a lightweight, isolated execution environment (Lua sandbox) on the terminal side.

## Architecture

In the terminal-side environment, the input pipeline is transformed into a programmable stream:
 
 - Source: Physical HID events (keyboard, mouse, focus, system signals) are captured by the terminal core as binary (Lua-aware) messages.
 - The Processor (Lua Sandbox): The application injects a script into the terminal's sandbox via an APC sequence. This script subscribes to specific event IDs (e.g., terminal.Listen("hids::keybd::any", ...)).
 - The Output: The script processes these events and emits custom-formatted data back to the application using an unified interface.

## Solving the Obfuscation Leakage

The primary goal of ADR is to eliminate side-channel vulnerabilities (Timing and Packet Size Analysis) that plague traditional protocols:
 - Constant Bitrate (CBR) Enforcing: The Lua script acts as a "metronome." It collects events and flushes them to the network at fixed intervals (e.g., every 20ms).
 - Deterministic Packet Size: By using some binary format, the script ensures that every outbound frame has a fixed length. If no input is detected, the script automatically generates payload::chaff (No-Op) frames.
 - Entropy Suppression: Because the "noise" (chaff) and "signal" (actual input) are formatted identically by the same script, an external observer (e.g., an attacker monitoring the SSH/network traffic) sees a monolithic, unchanging data stream with zero information leakage.

## Key Advantages

 • Protocol Agnosticism: The terminal no longer needs to be updated to support new input standards. If an application needs a new way to report 3D mouse coordinates or pressure-sensitive keys, it simply pushes a new Lua handler.
 • True Sandbox Isolation: The Lua environment is strictly isolated with no access to system APIs or direct network sockets. It can only communicate via the terminal.Send abstraction, ensuring that ADR cannot be weaponized to exfiltrate local data.
 • Bidirectional Integrity: Modern shells and TUIs can coordinate with the terminal to establish a "Full-Duplex Constant Stream," where the server-side "Pongs" every "Ping" from the terminal's ADR script, masking server response times.

## Notes

The ADR implementation must ensure that the transition between the default input mode and the scriptable mode is atomic. Once the hids subscription is active in the sandbox, raw terminal sequences should be suppressed to prevent "double-reporting" and metadata leaks.

## Example

### ADR Initiation Script (Lua) 

This script demonstrates how an application can "push" logic into the terminal sandbox to activate ADR mode with built-in obfuscation. It transforms the input stream into a protected, constant-rate flow. 

```lua
-- ADR Obfuscation Script for terminal
-- Purpose: Intercept keyboard events and enforce a 20ms Constant Bitrate (CBR) stream.

local event_buffer = {}
local FRAME_SIZE = 64  -- Fixed binary frame size
local TICK_RATE = 20   -- Transmission interval in milliseconds (50 FPS)

-- 1. Subscribe to keyboard events (ADR)
-- We intercept all keystrokes and store them in a local buffer.
terminal.Listen("hids::keybd::any", function(e)
    -- In terminal, 'e' is already a structured binary object/binary string.
    -- We push it into our queue for synchronized transmission.
    table.insert(event_buffer, e)
end)

-- 2. Chaff Generation Function (Noise)
-- Creates a dummy NOP packet that is indistinguishable from real input in size.
local function generate_chaff()
    -- Returns a binary NOP (No-Operation) message padded to FRAME_SIZE.
    return string.rep("\0", FRAME_SIZE) 
end

-- 3. Main Obfuscation Loop (Metronome)
-- Executed strictly every 20ms.
terminal.SetTimer(TICK_RATE, function()
    local payload = ""

    if #event_buffer > 0 then
        -- Pop the real event from the queue
        local raw_event = table.remove(event_buffer, 1)
        
        -- Apply Padding to ensure the packet size doesn't leak the event type.
        -- Whether it's "Enter" or "Ctrl+Alt+F12", the size remains constant.
        payload = pad_to_size(raw_event, FRAME_SIZE)
    else
        -- If no user activity, send a Chaff (noise) packet.
        payload = generate_chaff()
    end

    -- Send the prepared binary frame.
    -- To an external observer, this call happens continuously with identical weight.
    terminal.Send(payload)
end)

-- Helper function: Padding
function pad_to_size(data, size)
    if #data >= size then
        return string.sub(data, 1, size)
    else
        return data .. string.rep("\0", size - #data)
    end
end

-- Notify the server-side application that ADR mode is now active.
terminal.Send("ADR_MODE_ACTIVE")
```

#### Key Technical Takeaways

 - Stealth: The terminal.SetTimer ensures that data packets are dispatched on a strict schedule, regardless of whether the user is typing or idle.
 - Binary Uniformity: By leveraging the binary format, even when pad_to_size is applied, the packet structure remains a valid frame for the server-side parser.
 - Zero Information Leakage: An attacker monitoring the SSH/network traffic will observe a stream of packets (~100-120 bytes including headers) every 20ms with no detectable spikes or pauses.
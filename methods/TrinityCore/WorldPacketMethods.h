/*
 * Copyright (C) 2010 - 2024 Eluna Lua Engine
 * <https://elunaluaengine.github.io/> This program is free software licensed
 * under GPL version 3 Please see the included DOCS/LICENSE.md for more
 * information
 */

#ifndef WORLDPACKETMETHODS_H
#define WORLDPACKETMETHODS_H

#include "../../../../game/Server/WorldPacket.h"

/***
 * A packet used to pass messages between the server and a client.
 *
 * Each packet has an opcode that determines the type of message being sent,
 *   e.g. if a CMSG_LOGOUT_REQUEST packet is sent to the server,
 *   the client has sent a message that its [Player] wants to logout.
 *
 * The packet can contain further data, the format of which depends on the
 * opcode.
 *
 * Inherits all methods from: none
 */
namespace LuaPacket {
/**
 * Returns the opcode of the [WorldPacket].
 *
 * @return uint16 opcode
 */
int GetOpcode(Eluna *E, WorldPacket *packet) {
  E->Push(packet->GetOpcode());
  return 1;
}

/**
 * Returns the size of the [WorldPacket].
 *
 * @return uint32 size
 */
int GetSize(Eluna *E, WorldPacket *packet) {
  E->Push(packet->size());
  return 1;
}

/**
 * Sets the opcode of the [WorldPacket] to the specified opcode.
 *
 * @param [Opcodes] opcode : see Opcodes.h for all known opcodes
 */
int SetOpcode(Eluna *E, WorldPacket *packet) {
  uint32 opcode = E->CHECKVAL<uint32>(2);
  if (opcode >= NUM_OPCODES)
    return luaL_argerror(E->L, 2, "valid opcode expected");

  packet->SetOpcode((Opcodes)opcode);
  return 0;
}

/**
 * Reads and returns a signed 8-bit integer value from the [WorldPacket].
 *
 * @return int8 value
 */
int ReadByte(Eluna *E, WorldPacket *packet) {
  int8 _byte;
  (*packet) >> _byte;
  E->Push(_byte);
  return 1;
}

// ... Skipping to ReadString replacement ...
// I can't skip easily in one block if they are far apart.
// SetOpcode is at line 49. ReadString/WriteString is at 170-ish.
// I will use multi_replace.

/**
 * Reads and returns an unsigned 8-bit integer value from the [WorldPacket].
 *
 * @return uint8 value
 */
int ReadUByte(Eluna *E, WorldPacket *packet) {
  uint8 _ubyte;
  (*packet) >> _ubyte;
  E->Push(_ubyte);
  return 1;
}

/**
 * Reads and returns a signed 16-bit integer value from the [WorldPacket].
 *
 * @return int16 value
 */
int ReadShort(Eluna *E, WorldPacket *packet) {
  int16 _short;
  (*packet) >> _short;
  E->Push(_short);
  return 1;
}

/**
 * Reads and returns an unsigned 16-bit integer value from the [WorldPacket].
 *
 * @return uint16 value
 */
int ReadUShort(Eluna *E, WorldPacket *packet) {
  uint16 _ushort;
  (*packet) >> _ushort;
  E->Push(_ushort);
  return 1;
}

/**
 * Reads and returns a signed 32-bit integer value from the [WorldPacket].
 *
 * @return int32 value
 */
int ReadLong(Eluna *E, WorldPacket *packet) {
  int32 _long;
  (*packet) >> _long;
  E->Push(_long);
  return 1;
}

/**
 * Reads and returns an unsigned 32-bit integer value from the [WorldPacket].
 *
 * @return uint32 value
 */
int ReadULong(Eluna *E, WorldPacket *packet) {
  uint32 _ulong;
  (*packet) >> _ulong;
  E->Push(_ulong);
  return 1;
}

/**
 * Reads and returns a single-precision floating-point value from the
 * [WorldPacket].
 *
 * @return float value
 */
int ReadFloat(Eluna *E, WorldPacket *packet) {
  float _val;
  (*packet) >> _val;
  E->Push(_val);
  return 1;
}

/**
 * Reads and returns a double-precision floating-point value from the
 * [WorldPacket].
 *
 * @return double value
 */
int ReadDouble(Eluna *E, WorldPacket *packet) {
  double _val;
  (*packet) >> _val;
  E->Push(_val);
  return 1;
}

/**
 * Reads and returns an unsigned 64-bit integer value from the [WorldPacket].
 *
 * @return ObjectGuid value : value returned as string
 */
int ReadGUID(Eluna *E, WorldPacket *packet) {
  uint64 guid;
  (*packet) >> guid;
  E->Push(ObjectGuid(guid));
  return 1;
}

/**
 * Reads and returns a string value from the [WorldPacket].
 *
 * @return string value
 */
int ReadString(Eluna *E, WorldPacket *packet) {
  std::string _val;
  (*packet) >> _val;
  E->Push(_val);
  return 1;
}

/**
 * Writes an unsigned 64-bit integer value to the [WorldPacket].
 *
 * @param ObjectGuid value : the value to be written to the [WorldPacket]
 */
int WriteGUID(Eluna *E, WorldPacket *packet) {
  ObjectGuid guid = E->CHECKVAL<ObjectGuid>(2);
  (*packet) << (uint64)guid;
  return 0;
}

/**
 * Writes a string to the [WorldPacket].
 *
 * @param string value : the string to be written to the [WorldPacket]
 */
int WriteString(Eluna *E, WorldPacket *packet) {
  std::string _val = E->CHECKVAL<std::string>(2);
  (*packet) << _val;
  return 0;
}

/**
 * Writes a signed 8-bit integer value to the [WorldPacket].
 *
 * @param int8 value : the int8 value to be written to the [WorldPacket]
 */
int WriteByte(Eluna *E, WorldPacket *packet) {
  int8 byte = E->CHECKVAL<int8>(2);
  (*packet) << byte;
  return 0;
}

/**
 * Writes an unsigned 8-bit integer value to the [WorldPacket].
 *
 * @param uint8 value : the uint8 value to be written to the [WorldPacket]
 */
int WriteUByte(Eluna *E, WorldPacket *packet) {
  uint8 byte = E->CHECKVAL<uint8>(2);
  (*packet) << byte;
  return 0;
}

/**
 * Writes a signed 16-bit integer value to the [WorldPacket].
 *
 * @param int16 value : the int16 value to be written to the [WorldPacket]
 */
int WriteShort(Eluna *E, WorldPacket *packet) {
  int16 _short = E->CHECKVAL<int16>(2);
  (*packet) << _short;
  return 0;
}

/**
 * Writes an unsigned 16-bit integer value to the [WorldPacket].
 *
 * @param uint16 value : the uint16 value to be written to the [WorldPacket]
 */
int WriteUShort(Eluna *E, WorldPacket *packet) {
  uint16 _ushort = E->CHECKVAL<uint16>(2);
  (*packet) << _ushort;
  return 0;
}

/**
 * Writes a signed 32-bit integer value to the [WorldPacket].
 *
 * @param int32 value : the int32 value to be written to the [WorldPacket]
 */
int WriteLong(Eluna *E, WorldPacket *packet) {
  int32 _long = E->CHECKVAL<int32>(2);
  (*packet) << _long;
  return 0;
}

/**
 * Writes an unsigned 32-bit integer value to the [WorldPacket].
 *
 * @param uint32 value : the uint32 value to be written to the [WorldPacket]
 */
int WriteULong(Eluna *E, WorldPacket *packet) {
  uint32 _ulong = E->CHECKVAL<uint32>(2);
  (*packet) << _ulong;
  return 0;
}

/**
 * Writes a 32-bit floating-point value to the [WorldPacket].
 *
 * @param float value : the float value to be written to the [WorldPacket]
 */
int WriteFloat(Eluna *E, WorldPacket *packet) {
  float _val = E->CHECKVAL<float>(2);
  (*packet) << _val;
  return 0;
}

/**
 * Writes a 64-bit floating-point value to the [WorldPacket].
 *
 * @param double value : the double value to be written to the [WorldPacket]
 */
int WriteDouble(Eluna *E, WorldPacket *packet) {
  double _val = E->CHECKVAL<double>(2);
  (*packet) << _val;
  return 0;
}

#if defined ELUNA_SKYFIRE
// ── SKYFIRE: escritura bit a bit (formato de paquetes de MoP 5.4.8) ─────────
// Los paquetes de cliente de MoP llevan campos de N bits y los GUID van
// repartidos en una mascara de bits (que bytes son distintos de cero) y una
// secuencia de bytes (cada uno XOR 1). Estos metodos reproducen exactamente
// ByteBuffer::WriteBit/WriteBits/FlushBits/WriteGuidMask/WriteGuidBytes para
// que un script pueda construir un paquete tal como lo leeria el manejador.

/**
 * Writes a single bit.
 *
 * @param boolean bit
 */
int WriteBit(Eluna *E, WorldPacket *packet) {
  bool bit = E->CHECKVAL<bool>(2);
  packet->WriteBit(bit ? 1 : 0);
  return 0;
}

/**
 * Writes the low `count` bits of `value`, most significant first.
 *
 * @param uint32 value
 * @param uint32 count : number of bits (1..32)
 */
int WriteBits(Eluna *E, WorldPacket *packet) {
  uint32 value = E->CHECKVAL<uint32>(2);
  uint32 count = E->CHECKVAL<uint32>(3);
  if (count < 1 || count > 32)
    return luaL_argerror(E->L, 3, "1..32 bits expected");
  packet->WriteBits(value, count);
  return 0;
}

/**
 * Flushes the pending bits to a full byte. Call before writing bytes after bits.
 */
int FlushBits(Eluna * /*E*/, WorldPacket *packet) {
  packet->FlushBits();
  return 0;
}

/**
 * Writes the GUID mask bits for the given byte offsets, in that order.
 *
 * @param ObjectGuid guid
 * @param ... : byte offsets 0..7
 */
int WriteGuidMask(Eluna *E, WorldPacket *packet) {
  ObjectGuid guid = E->CHECKVAL<ObjectGuid>(2);
  int n = lua_gettop(E->L);
  for (int i = 3; i <= n; ++i) {
    uint32 off = E->CHECKVAL<uint32>(i);
    if (off > 7)
      return luaL_argerror(E->L, i, "byte offset 0..7 expected");
    packet->WriteBit(guid[off]);
  }
  return 0;
}

/**
 * Writes the GUID bytes (each XOR 1, skipped when zero) for the given offsets.
 *
 * @param ObjectGuid guid
 * @param ... : byte offsets 0..7
 */
int WriteGuidBytes(Eluna *E, WorldPacket *packet) {
  ObjectGuid guid = E->CHECKVAL<ObjectGuid>(2);
  int n = lua_gettop(E->L);
  for (int i = 3; i <= n; ++i) {
    uint32 off = E->CHECKVAL<uint32>(i);
    if (off > 7)
      return luaL_argerror(E->L, i, "byte offset 0..7 expected");
    packet->WriteByteSeq(guid[off]);
  }
  return 0;
}

/**
 * Writes raw bytes (no length, no terminator). Use after WriteBits(len, n).
 *
 * @param string bytes
 */
int WriteBytes(Eluna *E, WorldPacket *packet) {
  size_t len = 0;
  const char *s = luaL_checklstring(E->L, 2, &len);
  packet->append((uint8 const *)s, len);
  return 0;
}
#endif

ElunaRegister<WorldPacket> PacketMethods[] = {
    // Getters
    {"GetOpcode", &LuaPacket::GetOpcode},
    {"GetSize", &LuaPacket::GetSize},

    // Setters
    {"SetOpcode", &LuaPacket::SetOpcode},

    // Readers
    {"ReadByte", &LuaPacket::ReadByte},
    {"ReadUByte", &LuaPacket::ReadUByte},
    {"ReadShort", &LuaPacket::ReadShort},
    {"ReadUShort", &LuaPacket::ReadUShort},
    {"ReadLong", &LuaPacket::ReadLong},
    {"ReadULong", &LuaPacket::ReadULong},
    {"ReadGUID", &LuaPacket::ReadGUID},
    {"ReadString", &LuaPacket::ReadString},
    {"ReadFloat", &LuaPacket::ReadFloat},
    {"ReadDouble", &LuaPacket::ReadDouble},

    // Writers
    {"WriteByte", &LuaPacket::WriteByte},
    {"WriteUByte", &LuaPacket::WriteUByte},
    {"WriteShort", &LuaPacket::WriteShort},
    {"WriteUShort", &LuaPacket::WriteUShort},
    {"WriteLong", &LuaPacket::WriteLong},
    {"WriteULong", &LuaPacket::WriteULong},
    {"WriteGUID", &LuaPacket::WriteGUID},
    {"WriteString", &LuaPacket::WriteString},
    {"WriteFloat", &LuaPacket::WriteFloat},
    {"WriteDouble", &LuaPacket::WriteDouble},
#if defined ELUNA_SKYFIRE
    {"WriteBit", &LuaPacket::WriteBit},
    {"WriteBits", &LuaPacket::WriteBits},
    {"FlushBits", &LuaPacket::FlushBits},
    {"WriteGuidMask", &LuaPacket::WriteGuidMask},
    {"WriteGuidBytes", &LuaPacket::WriteGuidBytes},
    {"WriteBytes", &LuaPacket::WriteBytes},
#endif
};
}; // namespace LuaPacket

#endif

/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  network.c - A module providing a set of tools to marshal, send and receive
 *              data packets over a network.
 *
 * */

#include "network.h"

/* Initialise the networking subsystem */
ATRA_API
void
atra_net_init(void)
{
    SDLNet_Init();
}

/* Shut down the networking subsystem */
ATRA_API
void
atra_net_stop(void)
{
    SDLNet_Quit();
}

/* Normalise endianness of the 16-bit `value` and write to the given address */
ATRA_API
void
atra_net_write_u16(u16 value, void *buffer)
{
    SDLNet_Write16(value, buffer);
}

/* Normalise endianness of the 32-bit `value` and write to the given address */
ATRA_API
void
atra_net_write_u32(u32 value, void *buffer)
{
    SDLNet_Write32(value, buffer);
}

/* Read 16-bit value from the given address, normalising it's endianness */
ATRA_API
u16
atra_net_read_u16(void *buffer)
{
    return SDLNet_Read16(buffer);
}

/* Read 32-bit value from the given address, normalising it's endianness */
ATRA_API
u32
atra_net_read_u32(void *buffer)
{
    return SDLNet_Read32(buffer);
}


/* Name resolution functions */

/* Sets contents of `addr` to the IP address that the `host` resolves to.
 * Returns 0 on success, -1 on error. */
ATRA_API
int
atra_net_resolve_host(IPaddress *addr, const char *host, u16 port)
{
    return SDLNet_ResolveHost(addr, host, port);
}

/* Returns pointer to string containing host name that `addr` resolves to.
 * Returns NULL on error. */
ATRA_API
const char *
atra_net_resolve_ip(IPaddress *addr)
{
    return SDLNet_ResolveIP(addr);
}


/* TCP functions */

/* Connect to the host and port in `addr` using TCP */
ATRA_API
TCPsocket
atra_net_tcp_open(IPaddress *addr)
{
    return SDLNet_TCP_Open(addr);
}

/* Disconnect the given socket, closing the TCP connection */
ATRA_API
void
atra_net_tcp_close(TCPsocket socket)
{
    SDLNet_TCP_Close(socket);
}

/* Accept an incoming connection on the given TCP socket */
ATRA_API
TCPsocket
atra_net_tcp_accept(TCPsocket socket)
{
    return SDLNet_TCP_Accept(socket);
}

/* Returns the IP address and port of the given TCP socket */
ATRA_API
IPaddress *
atra_net_tcp_getpeeraddr(TCPsocket socket)
{
    return SDLNet_TCP_GetPeerAddress(socket);
}

/* Send data of a given length over the given TCP socket */
ATRA_API
int
atra_net_tcp_send(TCPsocket socket, const void *data, int length)
{
    return SDLNet_TCP_Send(socket, data, length);
}

/* Reveive data of exactly `length` bytes into the given `data` address */
ATRA_API
int
atra_net_tcp_recv(TCPsocket socket, void *data, int length)
{
    return SDLNet_TCP_Recv(socket, data, length);
}


/* UDP functions */

/* Open a UDP socket on the given port */
ATRA_API
UDPsocket
atra_net_udp_open(u16 port)
{
    return SDLNet_UDP_Open(port);
}

/* Close the given UDP socket */
ATRA_API
void
atra_net_udp_close(UDPsocket socket)
{
    SDLNet_UDP_Close(socket);
}

/* Bind an IP address to a socket channel (-1 for first available channel) */
ATRA_API
int
atra_net_udp_bind(UDPsocket socket, int channel, IPaddress *addr)
{
    return SDLNet_UDP_Bind(socket, channel, addr);
}

/* Unbind an IP address from a socket channel */
ATRA_API
void
atra_net_udp_unbind(UDPsocket socket, int channel)
{
    SDLNet_UDP_Unbind(socket, channel);
}

/* Returns the first IP address bound to a channel */
ATRA_API
IPaddress *
atra_net_udp_getpeeraddr(UDPsocket socket, int channel)
{
    return SDLNet_UDP_GetPeerAddress(socket, channel);
}

/* Send a packet using the specified socket (must be bound to target address).
 * Returns the number of destinations sent to that worked (returns 0 on error). */
ATRA_API
int
atra_net_udp_send(UDPsocket socket, int channel, UDPpacket *packet)
{
    return SDLNet_UDP_Send(socket, channel, packet);
}

/* Receive a packet on the specified socket into `packet`.
 * `packet` must have appropriate size pre-allocated.
 * Returns number of packets received (max 1 in this case), -1 on error. */
ATRA_API
int
atra_net_udp_recv(UDPsocket socket, UDPpacket *packet)
{
    return SDLNet_UDP_Recv(socket, packet);
}

/* Send a vector of packets */
ATRA_API
int
atra_net_udp_send_v(UDPsocket socket, UDPpacket **packets, int n)
{
    return SDLNet_UDP_SendV(socket, packets, n);
}

/* Receive a vector of packets */
ATRA_API
int
atra_net_udp_recv_v(UDPsocket socket, UDPpacket **packets)
{
    return SDLNet_UDP_RecvV(socket, packets);
}


/* UDP packet functions */

/* Allocate memory for a new UDP packet with a buffer of `size` bytes */
ATRA_API
UDPpacket *
atra_net_packet_create(int size)
{
    return SDLNet_AllocPacket(size);
}

/* Resize a UDP packet buffer to `size` bytes.
 * Returns the new size of the UDP packet. */
ATRA_API
int
atra_net_packet_resize(UDPpacket *packet, int size)
{
    return SDLNet_ResizePacket(packet, size);
}

/* Deallocate the UDP packet */
ATRA_API
void
atra_net_packet_delete(UDPpacket *packet)
{
    SDLNet_FreePacket(packet);
}

/* Allocate `count` UDP packets, each with a buffer of `size` bytes.
 * Returned packet vector is NULL terminated. */
ATRA_API
UDPpacket **
atra_net_packet_create_v(int count, int size)
{
    return SDLNet_AllocPacketV(count, size);
}

/* Deallocate a vector of `count` UDP packets */
ATRA_API
void
atra_net_packet_delete_v(UDPpacket **packets)
{
    SDLNet_FreePacketV(packets);
}


/* Socket set functions */

/* Allocate space for a set of `n` sockets to be monitored */
ATRA_API
SocketSet
atra_net_socketset_create(int n)
{
    return SDLNet_AllocSocketSet(n);
}

/* Deallocates a set of sockets */
ATRA_API
void
atra_net_socketset_delete(SocketSet set)
{
    SDLNet_FreeSocketSet(set);
}

/* Add a generic socket to a socket set that will be watched.
 * Returns number of spaces used, returns -1 on error. */
ATRA_API
int
atra_net_socketset_add(SocketSet set, GenericSocket socket)
{
    return SDLNet_AddSocket(set, socket);
}

/* Add a TCP socket to a socket set that will be watched.
 * Returns number of spaces used, returns -1 on error. */
ATRA_API
int
atra_net_socketset_addtcp(SocketSet set, TCPsocket socket)
{
    return SDLNet_TCP_AddSocket(set, socket);
}

/* Add a UDP socket to a socket set that will be watched.
 * Returns number of spaces used, returns -1 on error. */
ATRA_API
int
atra_net_socketset_addudp(SocketSet set, UDPsocket socket)
{
    return SDLNet_UDP_AddSocket(set, socket);
}

/* Remove a generic socket from a socket set (use before closing a watched socket).
 * Returns number of spaces used, returns -1 on error. */
ATRA_API
int
atra_net_socketset_remove(SocketSet set, GenericSocket socket)
{
    return SDLNet_DelSocket(set, socket);
}

/* Remove a generic socket from a socket set (use before closing a watched socket).
 * Returns number of spaces used, returns -1 on error. */
ATRA_API
int
atra_net_socketset_removetcp(SocketSet set, TCPsocket socket)
{
    return SDLNet_TCP_DelSocket(set, socket);
}

/* Remove a generic socket from a socket set (use before closing a watched socket).
 * Returns number of spaces used, returns -1 on error. */
ATRA_API
int
atra_net_socketset_removeudp(SocketSet set, UDPsocket socket)
{
    return SDLNet_UDP_DelSocket(set, socket);
}

/* Check all sockets in the socket set for activity (any change or action).
 * Waits `timeout` milliseconds for activity, 0 means no waiting.
 * Returns number of sockets with activity, returns -1 on error. */
ATRA_API
int
atra_net_socketset_checksockets(SocketSet set, u32 timeout)
{
    return SDLNet_CheckSockets(set, timeout);
}

/* Check if a socket has been marked as active.
 * Returns non-zero on discovered activity. */
#define atra_net_socketset_isready(socket) SDLNet_SocketReady(socket)

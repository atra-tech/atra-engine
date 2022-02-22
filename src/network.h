/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved
 *
 *  network.h - A module providing a set of tools to marshal, send and receive
 *              data packets over a network.
 *
 * */

#ifndef ATRA_NETWORK_H
#define ATRA_NETWORK_H

#include "common.h"

#include <SDL2/SDL_net.h>

#define ATRA_UDP_MAXCHANNELS    SDLNET_MAX_UDPCHANNELS
#define ATRA_UDP_MAXADDRESSES   SDLNET_MAX_UDPADDRESSES

#define ATRA_INADDR_ANY         INADDR_ANY
#define ATRA_INADDR_BROADCAST   INADDR_BROADCAST
#define ATRA_INADDR_LOOPBACK    INADDR_LOOPBACK
#define ATRA_INADDR_NONE        INADDR_NONE

typedef SDLNet_SocketSet SocketSet;
typedef SDLNet_GenericSocket GenericSocket;

/* Initialise the networking subsystem */
ATRA_API void atra_net_init(void);

/* Shut down the networking subsystem */
ATRA_API void atra_net_stop(void);

/* Normalise endianness of the 16-bit `value` and write to the given address */
ATRA_API void atra_net_write_u16(u16 value, void *buffer);

/* Normalise endianness of the 32-bit `value` and write to the given address */
ATRA_API void atra_net_write_u32(u32 value, void *buffer);

/* Read 16-bit value from the given address, normalising it's endianness */
ATRA_API u16 atra_net_read_u16(void *buffer);

/* Read 32-bit value from the given address, normalising it's endianness */
ATRA_API u32 atra_net_read_u32(void *buffer);


/* Name resolution functions */

/* Sets contents of `addr` to the IP address that the `host` resolves to.
 * Returns 0 on success, -1 on error. */
ATRA_API int atra_net_resolve_host(IPaddress *addr, const char *host, u16 port);

/* Returns pointer to string containing host name that `addr` resolves to.
 * Returns NULL on error. */
ATRA_API const char * atra_net_resolve_ip(IPaddress *addr);


/* TCP functions */

/* Connect to the host and port in `addr` using TCP */
ATRA_API TCPsocket atra_net_tcp_open(IPaddress *addr);

/* Disconnect the given socket, closing the TCP connection */
ATRA_API void atra_net_tcp_close(TCPsocket socket);

/* Accept an incoming connection on the given TCP socket */
ATRA_API TCPsocket atra_net_tcp_accept(TCPsocket socket);

/* Returns the IP address and port of the given TCP socket */
ATRA_API IPaddress * atra_net_tcp_getpeeraddr(TCPsocket socket);

/* Send data of a given length over the given TCP socket */
ATRA_API int atra_net_tcp_send(TCPsocket socket, const void *data, int length);

/* Reveive data of exactly `length` bytes into the given `data` address */
ATRA_API int atra_net_tcp_recv(TCPsocket socket, void *data, int length);


/* UDP functions */

/* Open a UDP socket on the given port */
ATRA_API UDPsocket atra_net_udp_open(u16 port);

/* Close the given UDP socket */
ATRA_API void atra_net_udp_close(UDPsocket socket);

/* Bind an IP address to a socket channel (-1 for first available channel) */
ATRA_API int atra_net_udp_bind(UDPsocket socket, int channel, IPaddress *addr);

/* Unbind an IP address from a socket channel */
ATRA_API void atra_net_udp_unbind(UDPsocket socket, int channel);

/* Returns the first IP address bound to a channel */
ATRA_API IPaddress * atra_net_udp_getpeeraddr(UDPsocket socket, int channel);

/* Send a packet using the specified socket (must be bound to target address).
 * Returns the number of destinations sent to that worked (returns 0 on error). */
ATRA_API int atra_net_udp_send(UDPsocket socket, int channel, UDPpacket *packet);

/* Receive a packet on the specified socket into `packet`.
 * `packet` must have appropriate size pre-allocated.
 * Returns number of packets received (max 1 in this case), -1 on error. */
ATRA_API int atra_net_udp_recv(UDPsocket socket, UDPpacket *packet);

/* Send a vector of packets */
ATRA_API int atra_net_udp_send_v(UDPsocket socket, UDPpacket **packets, int n);

/* Receive a vector of packets */
ATRA_API int atra_net_udp_recv_v(UDPsocket socket, UDPpacket **packets);


/* UDP packet functions */

/* Allocate memory for a new UDP packet with a buffer of `size` bytes */
ATRA_API UDPpacket * atra_net_packet_create(int size);

/* Resize a UDP packet buffer to `size` bytes.
 * Returns the new size of the UDP packet. */
ATRA_API int atra_net_packet_resize(UDPpacket *packet, int size);

/* Deallocate the UDP packet */
ATRA_API void atra_net_packet_delete(UDPpacket *packet);

/* Allocate `count` UDP packets, each with a buffer of `size` bytes.
 * Returned packet vector is NULL terminated. */
ATRA_API UDPpacket ** atra_net_packet_create_v(int count, int size);

/* Deallocate a vector of `count` UDP packets */
ATRA_API void atra_net_packet_delete_v(UDPpacket **packets);


/* Socket set functions */

/* Allocate space for a set of `n` sockets to be monitored */
ATRA_API SocketSet atra_net_socketset_create(int n);

/* Deallocates a set of sockets */
ATRA_API void atra_net_socketset_delete(SocketSet set);

/* Add a generic socket to a socket set that will be watched.
 * Returns number of spaces used, returns -1 on error. */
ATRA_API int atra_net_socketset_add(SocketSet set, GenericSocket socket);

/* Add a TCP socket to a socket set that will be watched.
 * Returns number of spaces used, returns -1 on error. */
ATRA_API int atra_net_socketset_addtcp(SocketSet set, TCPsocket socket);

/* Add a UDP socket to a socket set that will be watched.
 * Returns number of spaces used, returns -1 on error. */
ATRA_API int atra_net_socketset_addudp(SocketSet set, UDPsocket socket);

/* Remove a generic socket from a socket set (use before closing a watched socket).
 * Returns number of spaces used, returns -1 on error. */
ATRA_API int atra_net_socketset_remove(SocketSet set, GenericSocket socket);

/* Remove a generic socket from a socket set (use before closing a watched socket).
 * Returns number of spaces used, returns -1 on error. */
ATRA_API int atra_net_socketset_removetcp(SocketSet set, TCPsocket socket);

/* Remove a generic socket from a socket set (use before closing a watched socket).
 * Returns number of spaces used, returns -1 on error. */
ATRA_API int atra_net_socketset_removeudp(SocketSet set, UDPsocket socket);

/* Check all sockets in the socket set for activity (any change or action).
 * Waits `timeout` milliseconds for activity, 0 means no waiting.
 * Returns number of sockets with activity, returns -1 on error. */
ATRA_API int atra_net_socketset_checksockets(SocketSet set, u32 timeout);

/* Check if a socket has been marked as active.
 * Returns non-zero on discovered activity. */
#define atra_net_socketset_isready(socket) SDLNet_SocketReady(socket)


#endif /* ATRA_NETWORK_H */

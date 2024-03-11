/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#pragma once

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <message/Models.h>
#if defined (MESSAGE_STREAM_HAS_RECEIVE_HASHMAP)
  #include <map>
  using receiveFunction=std::function<void (Message*)>;
#endif

#include <functional>
using upstreamFunction=std::function<void(Message*)>;

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class MessageStream {
public:
    MessageStream(upstreamFunction upstream):
    upstream(upstream) {}

    /**
     * Send message upstream
     * @param m: message to send
     */
    virtual inline void sendUpstream(Message* m) { upstream(m); }

#if defined (MESSAGE_STREAM_HAS_RECEIVE_HASHMAP)
    /**
     * Send a message downstream to the handler capable of managing that message.
     * @param m: the message to be sent down stream
     * @param channel: the channel onto which sending the message
     */
    virtual inline void send(Message* m, std::string channel) {
        // try { // FIXME exception may be disabled on some cores
            receivers.at(channel)(m);
        // } catch(std::out_of_range&) {
            // the channel doesn't exist, ignore it
            // TODO report a warning message
        // }
    (())}

    /**
     * Add a receive function on the specified channel. If another receive function exists it will be replaced
     * @param channel: the channel onto which the receive function will be bound to
     * @param r: the receive function that will handle an incoming message
     */
    virtual inline void addReceive(std::string channel, receiveFunction r) { receivers[channel] = r; }

    /**
     * Remove a receive function that is bound to the provided channel
     * @param channel: the channel to empty
     */
    virtual inline void removeReceive(std::string channel) { receivers.erase(channel); }
#endif

private:
#if defined (MESSAGE_STREAM_HAS_RECEIVE_HASHMAP)
    std::map<std::string, receiveFunction> receivers;
#endif
    upstreamFunction upstream;
};

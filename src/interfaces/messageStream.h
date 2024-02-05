#pragma once
#include <models/models.h>
#include <map>
#include <functional>

using receiveFunction=std::function<void (Message*)>;
using upstreamFunction=std::function<void(Message*)>;

class MessageStream {
public:
    MessageStream(upstreamFunction upstream):
    upstream(upstream) {}

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
    }

    /**
     * Send message upstream
     * @param m: message to send
     */
    virtual inline void sendUpstream(Message* m) { upstream(m); }

    /**
     * Add a receve function on the specified channel. If another receive function exists it will be replaced
     * @param channel: the channel onto which the receive function will be bound to
     * @param r: the receive function that will handle an incoming message
     */
    virtual inline void addReceive(std::string channel, receiveFunction r) { receivers[channel] = r; }

    /**
     * Remove a receive function that is bound to the provided channel
     * @param channel: the channel to empty
     */
    virtual inline void removeReceive(std::string channel) { receivers.erase(channel); }

private:
    std::map<std::string, receiveFunction> receivers;
    upstreamFunction upstream;
};

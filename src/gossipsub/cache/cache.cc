#include <omnetpp.h>
#include "../packets_m.h"
#include "../internal_messages_m.h"
#include "cache.h"

using namespace omnetpp;


Define_Module(Cache);


void Cache::initialize() {
    node_id = par("nodeId").intValue();
    new_gossip_received_signal = registerSignal("newGossipReceived");
}

void Cache::handleMessage(cMessage *msg) {
    if (msg->arrivedOn("addGossipInputs")) {
        handleAddGossip(check_and_cast<Gossip *>(msg));
    } else if (msg->arrivedOn("queryPorts$i")) {
        handleQuery(check_and_cast<CacheQuery *>(msg));
    } else {
        error("unhandled message");
    }

    delete msg;
}

void Cache::handleAddGossip(Gossip *msg) {
    std::set<int> new_content_ids;

    // insert content Ids and note which ones are new
    int n = msg->getContentIdsArraySize();
    for (int i = 0; i < n; i++) {
        int content_id = msg->getContentIds(i);
        if (content_ids.find(content_id) == content_ids.end()) {
            // new entry
            content_ids.insert(content_id);
            new_content_ids.insert(content_id);
            emit(new_gossip_received_signal, msg->getHops());
        }
    }

    if (!new_content_ids.empty()) {
        // send new gossip to corresponding outputs
        Gossip *new_gossip_msg = new Gossip();
        // keeping the original sender signals to gardener that it should not send it back to the
        // relayer
        new_gossip_msg->setSender(msg->getSender());
        new_gossip_msg->setContentIdsArraySize(new_content_ids.size());
        new_gossip_msg->setHops(msg->getHops() + 1);

        int i = 0;
        for (auto content_id : new_content_ids) {
            new_gossip_msg->setContentIds(i, content_id);
            i++;
        }

        int gate_base_id = gateBaseId("newGossipOutputs");
        for (int i = 0; i < gateSize("newGossipOutputs"); i++) {
            send(new_gossip_msg->dup(), gate_base_id + i);
        }

        delete new_gossip_msg;
    }

    // make sender eager or lazy depending on if the message is new or not (unless its coming
    // from source)
    if (msg->getSender() != node_id) {
        GardenerControl *gardener_control = new GardenerControl();
        int sender = msg->getSender();
        if (new_content_ids.empty()) {
            // make sender lazy
            gardener_control->setPruneReceiversArraySize(1);
            gardener_control->setPruneReceivers(0, sender);
        } else {
            // make sender eager
            gardener_control->setGraftReceiversArraySize(1);
            gardener_control->setGraftReceivers(0, sender);
        }
        send(gardener_control, "gardenerControlOutput");
    }
}

void Cache::handleQuery(CacheQuery *msg) {
    int content_id = msg->getContentId();
    bool found = content_ids.find(content_id) != content_ids.end();

    CacheQueryResponse *response = new CacheQueryResponse();
    response->setFound(found);

    cGate *arrival_gate = msg->getArrivalGate();
    int response_gate_id = gateBaseId("queryPorts$o") + arrival_gate->getIndex();
    send(response, response_gate_id);
}

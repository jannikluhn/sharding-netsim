#include "overlay_manager.h"
#include <algorithm>
#include "../../packets_m.h"

using namespace omnetpp;


Define_Module(OverlayManager);

void OverlayManager::initialize() {
    heartbeat_interval = par("heartbeatInterval").doubleValue();
    target_mesh_degree = par("targetMeshDegree").intValue();
    low_watermark = par("lowWatermark").intValue();
    high_watermark = par("highWatermark").intValue();

    overlay_changed_signal = registerSignal("overlayChanged");

    cMessage *heartbeat = new cMessage();
    scheduleAt(simTime() + heartbeat_interval, heartbeat);
}


void OverlayManager::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleHeartbeat(msg);
    } else if (msg->arrivedOn("graftInput")) {
        handleGraft(check_and_cast<Graft *>(msg));
    } else if (msg->arrivedOn("pruneInput")) {
        handlePrune(check_and_cast<Prune *>(msg));
    } else if (msg->arrivedOn("peerListChangeInput")) {
        handlePeerListChange(check_and_cast<PeerListChange *>(msg));
    } else {
        error("unhandled message");
    }
}

void OverlayManager::handleHeartbeat(cMessage *heartbeat) {
    if (mesh_peers.size() < low_watermark) {
        EV_DEBUG << "not enough mesh peers (" << mesh_peers.size() << " < " << low_watermark
            << ")" << endl;
        while (mesh_peers.size() < target_mesh_degree) {
            if (non_mesh_peers.size() == 0) {
                EV_WARN << "no more active peers available" << endl;
                break;
            } else {
                int non_mesh_peer_index = intuniform(0, non_mesh_peers.size() - 1);
                int new_peer = non_mesh_peers[non_mesh_peer_index];

                mesh_peers.push_back(new_peer);
                non_mesh_peers.erase(non_mesh_peers.begin() + non_mesh_peer_index);

                EV_DEBUG << "grafted connection to " << new_peer << " (got " << mesh_peers.size()
                    << " peers now)" << endl;
                emit(overlay_changed_signal, mesh_peers.size());

                Graft *graft = new Graft();
                graft->setReceiver(new_peer);
                send(graft, "out");
            }
        }
    } else if (mesh_peers.size() > high_watermark) {
        EV_DEBUG << "too many mesh peers (" << mesh_peers.size() << " > " << high_watermark << ")"
            << endl;
        while (mesh_peers.size() > target_mesh_degree) {
            int mesh_peer_index = intuniform(0, mesh_peers.size() - 1);
            int peer = mesh_peers[mesh_peer_index];

            mesh_peers.erase(mesh_peers.begin() + mesh_peer_index);
            non_mesh_peers.push_back(peer);

            EV_DEBUG << "pruned connection to " << peer << " (got " << mesh_peers.size()
                << " peers now)" << endl;
            emit(overlay_changed_signal, mesh_peers.size());

            Prune *prune = new Prune();
            prune->setReceiver(peer);
            send(prune, "out");
        }
    }

    scheduleAt(simTime() + heartbeat_interval, heartbeat);
}

//
// Keep peer list in sync with connection manager
//
void OverlayManager::handlePeerListChange(PeerListChange *peer_list_change) {
    for (int i = 0; i < peer_list_change->getAddedPeersArraySize(); i++) {
        int added_peer = peer_list_change->getAddedPeers(i);
        if (isPeer(added_peer)) {
            error("tried adding peer more than once");
        } else {
            non_mesh_peers.push_back(added_peer);
            EV_DEBUG << "added new active peer " << added_peer << " as non-mesh peer" << endl;
        }
    }

    for (int i = 0; i < peer_list_change->getRemovedPeersArraySize(); i++) {
        int removed_peer = peer_list_change->getRemovedPeers(i);
        if (isMeshPeer(removed_peer)) {
            mesh_peers.erase(
                std::remove(mesh_peers.begin(), mesh_peers.end(), removed_peer),
                mesh_peers.end()
            );
            EV_DEBUG << "removed mesh peer " << removed_peer << " due to disconnect" << endl;
            emit(overlay_changed_signal, mesh_peers.size());
        } else if (isNonMeshPeer(removed_peer)) {
            non_mesh_peers.erase(
                std::remove(non_mesh_peers.begin(), non_mesh_peers.end(), removed_peer),
                non_mesh_peers.end()
            );
            EV_DEBUG << "removed non mesh peer " << removed_peer << " due to disconnect" << endl;
            emit(overlay_changed_signal, mesh_peers.size());
        } else {
            error("Tried removing node that is not a peer");
        }
    }

    delete peer_list_change;
}


//
// Graft and prune handlers
//
void OverlayManager::handleGraft(Graft *graft) {
    int peer = graft->getSender();
    if (!isPeer(peer)) {
        EV_ERROR << "received GRAFT from " << peer << " who is not a peer" << endl;
        error("received GRAFT from unknown peer");
    }

    if (!isMeshPeer(peer)) {
        mesh_peers.push_back(peer);
        non_mesh_peers.erase(
            std::remove(non_mesh_peers.begin(), non_mesh_peers.end(), peer),
            non_mesh_peers.end()
        );
        EV_DEBUG << "grafted connection with " << peer << " as requested by them" << endl;
        emit(overlay_changed_signal, mesh_peers.size());
    } else {
        // might happen if both send GRAFT at the same time
        EV_DEBUG << "ignored GRAFT from " << peer << " as they are mesh peers already" << endl;
    }

    delete graft;
}

void OverlayManager::handlePrune(Prune *prune) {
    int peer = prune->getSender();
    if (!isPeer(peer)) {
        EV_ERROR << "received PRUNE from " << peer << " who is not a peer" << endl;
        error("received PRUNE from unknown peer");
    }

    if (isMeshPeer(peer)) {
        mesh_peers.erase(
            std::remove(mesh_peers.begin(), mesh_peers.end(), peer),
            mesh_peers.end()
        );
        non_mesh_peers.push_back(peer);
        EV_DEBUG << "pruned connection with " << peer << " as requested by them" << endl;
        emit(overlay_changed_signal, mesh_peers.size());
    } else {
        // might happen if both send PRUNE at the same time
        EV_DEBUG << "ignored PRUNE from " << peer << " as the connection is pruned already"
            << endl;
    }

    delete prune;
}


//
// Utils
//
bool OverlayManager::isNonMeshPeer(int node_id) {
    return std::find(
        non_mesh_peers.begin(),
        non_mesh_peers.end(),
        node_id
    ) != non_mesh_peers.end();
}

bool OverlayManager::isMeshPeer(int node_id) {
    return std::find(mesh_peers.begin(), mesh_peers.end(), node_id) != mesh_peers.end();
}

bool OverlayManager::isPeer(int node_id) {
    return isMeshPeer(node_id) || isNonMeshPeer(node_id);
}

std::vector<int> OverlayManager::shuffle(std::vector<int> v) {
    // shuffle manually so that omnet++'s RNGs are used (see
    // https://en.wikipedia.org/wiki/Fisher%E2%80%93Yates_shuffle#The_modern_algorithm)
    for (int i = v.size() - 1; i >= 1; i--) {
        int j = intuniform(0, i);
        int value = v[j];
        v[j] = v[i];
        v[i] = value;
    }
    return v;
}

std::vector<int> OverlayManager::getNonMeshPeerShuffling() {
    Enter_Method_Silent();

    return shuffle(non_mesh_peers);
}

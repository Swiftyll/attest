#include "comp2.hpp"
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include <string>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cassert>

#include "src/pb2/phase2.pb.h"

void *worker_routine (void *arg)
{
    zmq::context_t *context = (zmq::context_t *) arg;

    zmq::socket_t socket (*context, ZMQ_REP);
    socket.connect ("inproc://workers");

    bool ok = true;

    while (true) {
        //  Wait for next request from client
        zmq::multipart_t mp_request;
        zmq::message_t msg;
        ok = mp_request.recv(socket);

        assert(mp_request.size() == 1);
        assert(ok);

        //   socket.recv (&request);
        Person p = Person();
        msg = mp_request.pop(); 
        std::cout << "size " << msg.size() << std::endl;
        p.ParseFromArray(msg.data(),msg.size());
        std::cout << p.name() << std::endl;
        std::cout << p.id() << std::endl;

        //std::cout << "Received request: [" << (char*) request.data() << "]" << std::endl;

        // Do some 'work'
        // sleep (1);

        // Send reply back to client
        zmq::message_t reply (6);
        memcpy ((void *) reply.data (), "World", 6);
        socket.send (reply);
    }
    return (NULL);
}

int Comp2::c2method( int input){
    //  Prepare our context and sockets
    zmq::context_t context (1);
    zmq::socket_t clients (context, ZMQ_ROUTER);
    clients.bind ("tcp://0.0.0.0:8123");
    zmq::socket_t workers (context, ZMQ_DEALER);
    workers.bind ("inproc://workers");

    //  Launch pool of worker threads
    for (int thread_nbr = 0; thread_nbr != 10; thread_nbr++) {
        pthread_t worker;
        pthread_create (&worker, NULL, worker_routine, (void *) &context);
    }
    //  Connect work threads to client threads via a queue
    zmq::proxy ((void *)clients, (void *)workers, NULL);

  return input * 4;

}

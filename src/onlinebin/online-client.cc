// onlinebin/online-net-client.cc

// Copyright 2012 Cisco Systems (author: Matthias Paulik)

//   Modifications to the original contribution by Cisco Systems made by:
//   Vassil Panayotov

// See ../../COPYING for clarification regarding multiple authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED
// WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE,
// MERCHANTABLITY OR NON-INFRINGEMENT.
// See the Apache 2 License for the specific language governing permissions and
// limitations under the License.

#include <netdb.h>
#include <fcntl.h>
#include "online/online-audio-source.h"
#include "online/online-feat-input.h"


int main(int argc, char *argv[]) {
  try {
    using namespace kaldi;

    typedef kaldi::int32 int32;

    // Time out interval for the PortAudio source
    const int32 kTimeout = 500; // half second
    // PortAudio sampling rate
    const int32 kSampleFreq = 16000;
    // PortAudio's internal ring buffer size in bytes
    const int32 kPaRingSize = 32768;
    // Report interval for PortAudio buffer overflows in number of feat. batches
    const int32 kPaReportInt = 4;

    const char *usage =
        "Takes input using a microphone(PortAudio), extracts features and sends them\n"
        "to a speech recognition server over a network connection\n\n"
        "Usage: online-net-client server-address server-port\n\n";
    ParseOptions po(usage);
    int32 batch_size = 27;
    po.Register("batch-size", &batch_size,
                "The number of feature vectors to be extracted and sent in one go");
    po.Read(argc, argv);
    if (po.NumArgs() != 2) {
      po.PrintUsage();
      return 1;
    }

    std::string server_addr_str = po.GetArg(1);
    std::string server_port_str = po.GetArg(2);

    addrinfo *server_addr, hints;
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_ADDRCONFIG;
    if (getaddrinfo(server_addr_str.c_str(), server_port_str.c_str(),
                    &hints, &server_addr) != 0)
      KALDI_ERR << "getaddrinfo() call failed!";
    int32 sock_desc = socket(server_addr->ai_family,
                             server_addr->ai_socktype,
                             server_addr->ai_protocol);
    if (sock_desc == -1)
      KALDI_ERR << "socket() call failed!";
    int32 flags = fcntl(sock_desc, F_GETFL);
    flags |= O_NONBLOCK;
    if (fcntl(sock_desc, F_SETFL, flags) == -1)
      KALDI_ERR << "fcntl() failed to put the socket in non-blocking mode!";

    
    int32 frame_length = 25;
    OnlinePaSource au_src(kTimeout, kSampleFreq, kPaRingSize, kPaReportInt);

    
    // Prepare the input audio samples
    int32 samples_req = frame_length * (kSampleFreq / 1000);

    Vector<BaseFloat> read_samples(samples_req);
    
    while (1) {
        au_src.Read(&read_samples);
        // std::cout << read_samples;
        std::stringstream ss;
        read_samples.Write(ss, false);
        std::cout << ss.str();
        ssize_t sent = sendto(sock_desc,
                              ss.str().c_str(),
                              ss.str().size(), 0,
                              server_addr->ai_addr,
                              server_addr->ai_addrlen);
        if (sent == -1)
          KALDI_ERR << "sendto() call failed!";
    }
    freeaddrinfo(server_addr);
    return 0;
  } catch(const std::exception& e) {
    std::cerr << e.what();
    return -1;
  }
} // main()

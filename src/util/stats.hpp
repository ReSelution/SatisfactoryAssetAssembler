//
// Created by oleub on 08.03.26.
//

#pragma once
#include "sc/stats/stats.hpp"
#include "sc/stats/throughput.hpp"

using DecompressedThroughput = sc::stats::Stat<"Decompressed", sc::stats::Throughput<>>;
using CompressedThroughput   = sc::stats::Stat<"Compressed", sc::stats::Throughput<>>;

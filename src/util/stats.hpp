//
// Created by oleub on 08.03.26.
//

#pragma once
#include "stats/chaos_stats.hpp"
#include "stats/chaos_throughput.hpp"

DEFINE_CHAOS_STAT(DecompressedThroughput, "Decompressed", SC::ChaosThroughput<>)
DEFINE_CHAOS_STAT(CompressedThroughput, "Compressed", SC::ChaosThroughput<>)

#!/bin/bash

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/wk/other/github/librdkafka/src

./out.bin $@

#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PATH_TO_PARENT=$DIR/streambox_release_March_10_2018
PATH_TO_SB=$PATH_TO_PARENT/streambox
TBB_INSTALL_PATH=/hdd1/enjima/tests/lsds-streambench/oneTBB
TBB_SYMLINK_PATH=$PATH_TO_PARENT/tbb

CMD_ARGS=$1

if [[ $CMD_ARGS == *"deps"* ]]; then
	echo "Installing Dependencies..."
	sudo apt-get install \
		g++ \
	     	libtbb-dev \
     		automake \
		autoconf \
		autoconf-archive \
		libtool \
		libboost-all-dev \
		libevent-dev \
		libdouble-conversion-dev \
		libgoogle-glog-dev \
		libgflags-dev \
		liblz4-dev \
		liblzma-dev \
		libsnappy-dev \
		make \
		zlib1g-dev \
		binutils-dev \
		libjemalloc-dev \
		libssl-dev;
fi

if [[ $CMD_ARGS == *"decp"* ]]; then
	echo "Downloading StreamBox..."
	wget ftp://ftp.ecn.purdue.edu/xzl/software/streambox/streambox-last.tar.gz
fi

if [[ $CMD_ARGS == *"ecp"* ]]; then
	tar -xvzf streambox-last.tar.gz 
	# tar -xvzf data.tar.gz
fi

if [ ! -L "$TBB_SYMLINK_PATH" ]; then
  ln -s  $TBB_INSTALL_PATH "$TBB_SYMLINK_PATH"
fi

if [[ $CMD_ARGS == *"cp"* ]]; then
	echo "Copying new Files..."
	Data_PATH=$DIR/data_test/Data.txt
	Campaigns_PATH=$DIR/data_test/CampAds.txt
	sed -i '65s#.*#"'$Data_PATH'"#' new_files/test-yahoo.cpp
	sed -i '75s#.*#"'$Data_PATH'"#' new_files/test-yahoo.cpp
	sed -i '98s#.*#"'$Campaigns_PATH'"#' new_files/test-yahoo.cpp

	cp new_files/Unbounded.cpp streambox_release_March_10_2018/streambox/Source/
	cp new_files/Unbounded.h streambox_release_March_10_2018/streambox/Source/
	cp new_files/UnboundedInMemEvaluator.h streambox_release_March_10_2018/streambox/Source/
	cp new_files/YahooMapper.cpp streambox_release_March_10_2018/streambox/Mapper/
	cp new_files/YahooMapper.h streambox_release_March_10_2018/streambox/Mapper/
	cp new_files/YahooMapperEvaluator.h streambox_release_March_10_2018/streambox/Mapper/
	cp new_files/Values.cpp streambox_release_March_10_2018/streambox/
	cp new_files/Values.h streambox_release_March_10_2018/streambox/
	cp new_files/test-yahoo.cpp streambox_release_March_10_2018/streambox/test/
	cp new_files/config.h streambox_release_March_10_2018/streambox/
	cp new_files/json.hpp streambox_release_March_10_2018/streambox/
	cp new_files/CMakeLists.txt streambox_release_March_10_2018/streambox/
	cp new_files/EvaluationBundleContext.h streambox_release_March_10_2018/streambox/core
	cp new_files/YahooBenchmarkSource.cpp streambox_release_March_10_2018/streambox/Source/
	cp new_files/YahooBenchmarkSource.h streambox_release_March_10_2018/streambox/Source/
	cp new_files/YahooBenchmarkSourceEvaluator.h streambox_release_March_10_2018/streambox/Source/
	cp new_files/SimpleSelect.h streambox_release_March_10_2018/streambox/Select/
	echo "Finished copying."
	echo
	sleep 2
fi

if [[ $CMD_ARGS == *"cmake"* ]]; then
	echo "Running CMAKE configuration..."
	cd "$PATH_TO_SB" || exit 1
	/usr/bin/cmake -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - Unix Makefiles" $PATH_TO_SB
	# make test-yahoo.bin
	echo
	sleep 2
fi

if [[ $CMD_ARGS == *"build"* ]]; then
	echo "Compiling benchmark..."
	cd "$PATH_TO_SB" || exit 1
	cmake --build . -j 12 --target test-yahoo.bin
	echo "Finished compilation."
	echo
	sleep 2
fi

if [[ $CMD_ARGS == *"skiprun"* ]]; then
	echo "Skipping execution..."
	echo "Done."
	exit 0
fi

echo "Running benchmark..."
cd "$PATH_TO_SB" || exit 1
./test-yahoo.bin

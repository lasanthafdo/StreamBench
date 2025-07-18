#include <Mapper/YahooMapper.h>
#include <Win/FixedWindowInto.h>
#include "core/Pipeline.h"

/* No need to include the transform headers.
 * Since the -Evaluator.h file already included the corresponding
 * transform header.
 */
#include "Source/UnboundedInMemEvaluator.h"
#include "Win/WinGBKEvaluator.h"
#include "WinKeyReducer/WinKeyReducerEval.h"
#include "Sink/WindowsBundleSinkEvaluator.h"
#include "test-common.h"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>

/*
 *      UnboundedInMem
 *            | BundleT<string_range>
 *            V
 *       WordCountMapper
 *            | BundleT<KVPair<string,long>>
 *            V
 *       WinGBK
 *            | WindowsKeyedBundle<KVPair<string,long>>
 *            V
 *      WindowKeyedReducer (stateful)
 *            | WindowsBundle<KVPair<string,long>>
 *            V
 *				WindowsBundle
 *
 *	 where @BundleT can be specified below.
 */


/* -- define the bundle type we use -- */
template<class T>
using BundleT = RecordBundle<T>;
//using BundleT = RecordBitmapBundle<T>;


/* default config. can override on cmdline options */
#ifdef DEBUG
pipeline_config config = {
		.records_per_interval = 400*1000,
		.target_tput = (800 * 1000),
		.record_size = 1000,
		.input_file = 
"/hdd1/enjima/tests/lsds-streambench/StreamBench/streambox/data_test/Data.txt"
        ,
		.cores = 8,//std::thread::hardware_concurrency() - 1,
};
#else
pipeline_config config = {
    .records_per_interval = (1000 * 1000),
    .target_tput = (CONFIG_SOURCE_THREADS * 12 * 1000 * 1000),
    .record_size = 48,
    .input_file =
    "/hdd1/enjima/tests/lsds-streambench/StreamBench/streambox/data_test/Data.txt",
    .cores = std::thread::hardware_concurrency() - 1,
};
#endif

void testYahooBenchmark() {
    using namespace boost::uuids;
    using KVPair = pair<uint64_t, creek::string>;
    using Set = creek_set_array::SetArray;
    using Vector = creek::concurrent_vector<creek::string>;

    unordered_map<uint64_t, creek::string> campaigns;
    campaigns.reserve(1000);

    std::ifstream infile(
        "/hdd1/enjima/tests/lsds-streambench/StreamBench/streambox/data_test/CampAds.txt"
    );
    std::string line;
    vector<string> myString;
    std::hash<std::string> str_hasher;
    int i = 0;
    while (std::getline(infile, line)) {
        istringstream ss(line);
        string token;
        while (getline(ss, token, ',')) {
            myString.push_back(token);
            i++;
        }
        //campaigns.insert(std::pair<uuid, uuid>(parse_uuid(myString[i-2]), parse_uuid(myString[i-1]))); //[parse_uuid(myString.at(i-2))] = parse_uuid(myString.at(i-1));
        campaigns.insert(KVPair(str_hasher(myString[i - 2]), myString[i - 1]));
        //[parse_uuid(myString.at(i-2))] = parse_uuid(myString.at(i-1));
    }
    myString.clear();

    UnboundedInMem<YSBEvent, BundleT> yahooSource(
        // YahooBenchmarkSource yahooSource(
        "[yahooSource]",
        config.input_file.c_str(),
        config.records_per_interval, /* records per wm interval */
        config.target_tput, /* target tput (rec/sec) */
        config.record_size,
        0 //XXX session_gap_ms = ????
    );

    // create a new pipeline
    Pipeline *p = Pipeline::create(NULL);

    // Source
    PCollection *yahooSource_output = dynamic_cast<PCollection *>(p->apply1(&yahooSource));
    yahooSource_output->_name = "src_out";


    // Without group by
    /*YahooMapper<Event, creek::string, BundleT> mapper ("[yahoo-mapper]", campaigns);
    FixedWindowInto<creek::string, BundleT> fwi ("window", seconds(1));*/
    /*WinSum_mergeset<creek::string, shared_ptr<Set>> agg ("agg", 10);
    RecordBundleSink<shared_ptr<Set>> sink("sink");*/
    /*WinSum_mergevector<creek::string, shared_ptr<Vector>> agg ("agg", 1);
    RecordBundleSink<shared_ptr<Vector>> sink("sink");
    connect_transform(yahooSource, mapper);
    connect_transform(mapper, fwi);
    connect_transform(fwi, agg);
    connect_transform(agg, sink);*/

    // With group by
    YahooMapper<YSBEvent, pair<creek::string, long>, BundleT> mapper("[yahoo-mapper]", campaigns);
    WinGBK<pair<creek::string, long>, BundleT, WinKeyFragLocal_Std> wgbk("[wingbk]", seconds(10));
    WinKeyReducer<pair<creek::string, long>, // pair in
        WinKeyFragLocal_Std, WinKeyFrag_Std, // kv d/s
        pair<creek::string, long>, // pair out
        WindowsBundle // bundle out
    > reducer("[reducer]");
    WindowsBundleSink<pair<creek::string, long> > sink("sink");
    connect_transform(yahooSource, mapper);
    connect_transform(mapper, wgbk);
    connect_transform(wgbk, reducer);
    connect_transform(reducer, sink);

    EE("config.cores=%lu", config.cores);
    // Eval the pipeline
    EvaluationBundleContext eval(1, config.cores);
    eval.runSimple(p);
}

//#include <jemalloc/jemalloc.h>

int main(int ac, char *av[]) {
    parse_options(ac, av, &config);

    //  malloc_stats_print(NULL, NULL, NULL);  // can test jemalloc
    print_config();
    testYahooBenchmark();

    return 0;
}

#include <thread>
#include <cppkafka/cppkafka.h>


namespace cppkafka {
const char* G_SERVERS = "nuc.cn:2004,nuc.cn:2005,nuc.cn:2006";
const char* G_QUEUE_NAME = "rd_test_queue";

void read(int emptyMax) {
    if (emptyMax < 1) {
        printf("read none\n");
        return;
    }
    bool offset_commit_called = false;
    Configuration config;
    config.set("metadata.broker.list", G_SERVERS);
    config.set("enable.auto.commit", false);
    // config.set("auto.offset.reset", "latest");
    config.set("group.id", "rd_group_debug");
    config.set_offset_commit_callback([&](Consumer&, Error error, const TopicPartitionList& tpartitions) {
        offset_commit_called = true;
        printf("--- set_offset_commit_callback: err=%s, partitions.size=%lu\n", error ? "true" : "false",
            tpartitions.size());
        for (size_t i = 0; i < tpartitions.size(); ++i) {
            printf("--- set_offset_commit_callback: [%lu],name=%s,part=%d,offset=%ld\n", i,
                tpartitions[i].get_topic().c_str(), tpartitions[i].get_partition(), tpartitions[i].get_offset());
        }
    });

    TopicPartitionList assignment;
    bool revocation_called = false;

    Consumer consumer(config);
    consumer.set_assignment_callback([&](const TopicPartitionList& tpartitions) {
        assignment = tpartitions;
        printf("--- consumer.set_assignment_callback: size=%lu\n", tpartitions.size());
        for (size_t i = 0; i < tpartitions.size(); ++i) {
            printf("--- consumer.set_assignment_callback: [%lu],name=%s,part=%d,offset=%ld\n", i,
                tpartitions[i].get_topic().c_str(), tpartitions[i].get_partition(), tpartitions[i].get_offset());
            const_cast<TopicPartitionList&>(tpartitions)[i].set_offset(TopicPartition::OFFSET_STORED);
        }
    });
    consumer.set_revocation_callback([&](const TopicPartitionList& tpartitions) {
        revocation_called = true;
        printf("--- consumer.set_revocation_callback: size=%lu\n", tpartitions.size());
        for (size_t i = 0; i < tpartitions.size(); ++i) {
            printf("--- consumer.set_revocation_callback: [%lu],name=%s,part=%d,offset=%ld\n", i,
                tpartitions[i].get_topic().c_str(), tpartitions[i].get_partition(), tpartitions[i].get_offset());
        }
    });
    std::chrono::milliseconds tout(500);
    consumer.set_timeout(std::chrono::milliseconds(500));
    consumer.subscribe({ G_QUEUE_NAME });
    bool running_ = true;
    int cnt = 0;
    int emptycnt = 0;
    while (running_ && emptycnt < emptyMax) {
        Message msg = consumer.poll();
        if (!msg) {
            ++emptycnt;
            // printf("msg[%d] = %d;\n", cnt, ++emptycnt);
            //  on_timeout(Timeout{});
        } else if (msg.get_error()) {
            printf("msg[%d] = err\n", cnt);
            if (msg.is_eof()) {
                // running_=false;
                // on_eof(EndOfFile{}, { msg.get_topic(), msg.get_partition(), msg.get_offset() });
            } else {
                // on_error(msg.get_error());
            }
        } else {
            printf("part=%d,offset=%ld, msg[%d] = %.*s\n", msg.get_partition(), msg.get_offset(), ++cnt,
                (int)msg.get_payload().get_size(), msg.get_payload().get_data());
            consumer.commit(msg);
            // consumer.store_offset(msg);
        }
    }
    // consumer.close();
    printf("msg=%d, none=%d, finished\n", cnt, emptycnt);
}


int write(int cnt) {
    int finishedmsg = 0;
    if (cnt < 1) {
        printf("write finished = %d/%d\n", finishedmsg, cnt);
        return 0;
    }
    Configuration config = {
        {"metadata.broker.list", G_SERVERS},
        {"acks", "1"},
        {"request.required.acks", "1"}
    };
    config.set_delivery_report_callback([&](Producer& producer, const Message& msg) {
        if (!msg.get_error()) {
            ++finishedmsg;
        } else {
            printf("set_delivery_report_callback: err=%s\n", msg.get_error().to_string().c_str());
        }
    });

    Producer producer(config);
    producer.set_timeout(std::chrono::milliseconds(200));
    printf("write cnt = %d\n", cnt);
    char buf[256];
    for (int i = 0; i < cnt; ++i) {
        snprintf(buf, sizeof(buf), "debugFlyMsg%d", i + 1);
        try {
            producer.produce(MessageBuilder(G_QUEUE_NAME).partition(0x7 & i).payload(buf));
        } catch (HandleException& err) {
            printf("write: idx=%d, catch err = %s\n", i, err.what());
            producer.poll();
            --i;
        }
    }
    for (; finishedmsg < cnt;) {
        printf("write finishedmsg = %d/%d\n", finishedmsg, cnt);
        producer.poll();
    }
    try {
        producer.flush(std::chrono::milliseconds(20));
    } catch (HandleException& err) {
        printf("flush: catch err = %s\n", err.what());
    }
    printf("write finished = %d/%d\n", finishedmsg, cnt);
    return 0;
}

} // namespace cppkafka

int main(int argc, char** argv) {
    int cnt = argc > 1 ? std::stoi(argv[1]) : 10;
    int emptycnt = argc > 2 ? std::stoi(argv[2]) : 100;
    printf("argc=%d, cnt = %d, emptycnt=%d\n", argc, cnt, emptycnt);
    cppkafka::write(cnt);
    cppkafka::read(emptycnt);
    return 0;
}
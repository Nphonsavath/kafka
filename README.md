# kafka

1. Project Overview
  - What is Kafka?
    - Distributed event streaming platform, handle continuous streams of data (events) such as user clicks, transactions, sensor readings, etc.
      - Distributed = Kafka is run across multiple brokers (servers), that work together as a cluster. Able to be horizontally scaled by adding more brokers to a cluster.
    - Kafka is the central message broker, where publishers publish data to topics, and consumers subscribe to these topics to receive data.
      - Topic = An organization of data, example a user activity topic for a website could have page views, user clicks, time spent, etc
      - 
  - Advantages
    - Fault-Tolerant
      - Data is replicated across multiple brokers, thus ensuring a reundancy mechanism
    - High throughput and low latency [1]
      - Partitions
        - Topics are split further into partitions across different brokers in the cluster, which allows read and writes in parallel. Each partition increases the systems throughput.
        - 
   
8. Resources
  - Why Kafka has high throughput
    - https://github.com/AutoMQ/automq/wiki/Why-is-Kafka-throughput-so-high [1]
  - 

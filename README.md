# kafka

1. Project Overview
  - What is Kafka?
    - Distributed event streaming platform, handle continuous streams of data (events) such as user clicks, transactions, sensor readings, etc.
      - Distributed = Kafka is run across multiple brokers (servers), that work together as a cluster. Able to be horizontally scaled by adding more brokers to a cluster.
    - Kafka is the central message broker, where publishers publish data to topics, and consumers subscribe to these topics to receive data.
      - Topic = An organization of data, example a user activity topic for a website could have page views, user clicks, time spent, etc
     
    - <img width="770" height="462" alt="image" src="https://github.com/user-attachments/assets/b0baf235-0121-4958-9d3d-57dff98208fe" /> {1}
 
  - Advantages
    - Fault-Tolerant
      - Data is replicated across multiple brokers, thus ensuring a reundancy mechanism
    - High throughput and low latency [1]
      - Partitions
        - Topics are split further into partitions across different brokers in the cluster, which allows read and writes in parallel. Each partition increases the systems throughput.
        - <img width="600" height="295" alt="image" src="https://github.com/user-attachments/assets/42689b21-cbc3-4bdb-b562-d195684e6663" /> {2}
      - Zero Copy
        - 
   
8. Resources
  - Why Kafka has high throughput
    - https://github.com/AutoMQ/automq/wiki/Why-is-Kafka-throughput-so-high [1]
    - https://www.nootcode.com/knowledge/en/kafka-zero-copy [2]
  - Kafka Protocol Guide
    - https://kafka.apache.org/protocol

9. Images
  - https://i0.wp.com/lab.wallarm.com/wp-content/uploads/2024/01/280-min.jpg?w=770&ssl=1 {1}
  - https://raw.githubusercontent.com/DXHeroes/knowledge-base-content/master/files/kafka_architecture.png {2}

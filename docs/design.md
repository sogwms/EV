# DESIGN

The main principle that EV follows is utility.

TODO DIAGRAM

## Pub-Sub

### Introduction to pub-sub pattern

[https://realtimeapi.io/hub/publishsubscribe-pattern/](https://realtimeapi.io/hub/publishsubscribe-pattern/)

[https://docs.microsoft.com/en-us/azure/architecture/patterns/publisher-subscriber](https://docs.microsoft.com/en-us/azure/architecture/patterns/publisher-subscriber)

### What does pub-sub look like in EV ?

The implementation of pub-sub is in the pss file. the latest version is 0.1.0


version：0.1.0
    
- single instance
- topic-based
- pre-defined topic and O(1) search
- 'dead' broker and serial distribution (which means when the publisher publishs a message, the cpu will be occupied by the broker until all message distributions are complete. and no buffer)

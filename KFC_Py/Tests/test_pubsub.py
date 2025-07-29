import pytest
from pubsub import Publisher, Subscriber

class TestSubscriber(Subscriber):
    def __init__(self):
        self.events = []
    def update(self, event_type, data):
        self.events.append((event_type, data))

def test_pubsub_basic():
    pub = Publisher()
    sub1 = TestSubscriber()
    sub2 = TestSubscriber()
    pub.subscribe('move', sub1)
    pub.subscribe('capture', sub1)
    pub.subscribe('move', sub2)

    pub.notify('move', {'piece': 'QW', 'from': (1,1), 'to': (2,2)})
    pub.notify('capture', {'attacker': 'QW', 'victim': 'PB'})

    assert sub1.events == [
        ('move', {'piece': 'QW', 'from': (1,1), 'to': (2,2)}),
        ('capture', {'attacker': 'QW', 'victim': 'PB'})
    ]
    assert sub2.events == [
        ('move', {'piece': 'QW', 'from': (1,1), 'to': (2,2)})
    ]

def test_unsubscribe():
    pub = Publisher()
    sub = TestSubscriber()
    pub.subscribe('move', sub)
    pub.notify('move', 1)
    pub.unsubscribe('move', sub)
    pub.notify('move', 2)
    assert sub.events == [('move', 1)]

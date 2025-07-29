class Subscriber:
    def update(self, event_type, data):
        """Override in subclass: handle event of type event_type with data."""
        pass

class Publisher:
    def __init__(self):
        self._subscribers = {}

    def subscribe(self, event_type, subscriber):
        if event_type not in self._subscribers:
            self._subscribers[event_type] = []
        self._subscribers[event_type].append(subscriber)

    def unsubscribe(self, event_type, subscriber):
        if event_type in self._subscribers:
            self._subscribers[event_type].remove(subscriber)

    def notify(self, event_type, data=None):
        for sub in self._subscribers.get(event_type, []):
            sub.update(event_type, data)

import time 

class Bench:
    def __init__(self, title=None):
        self.title = title
        self.time_start = None
        self.time_end = None
        self.duration = None

    def reset(self):
        self.time_end = None
        self._duration = None
        self.time_start = time.time()

    def __enter__(self):
        self.time_start = time.time()
        print(f"Benching {self.title}.")
        return self

    @property
    def duration(self):
        if not self._duration:
            self.time_end = time.time()
            self._duration = time.time() - self.time_start
        return self._duration

    @duration.setter
    def duration(self, value):
        self._duration = value

    def __exit__(self, *args):
        self.duration
        print(f"-> {self.title} took {self.duration.__round__(4)} seconds.")


# Complex type alias with multiple type params (Python 3.12+)
type Handler[T, **P, R] = Callable[P, R]

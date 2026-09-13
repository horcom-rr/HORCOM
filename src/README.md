# src

The new C++ implementation. Module layout, dependency rules and porting order are defined in `docs/architecture.md`; the subsystem maps in `docs/legacy/` document the original the port follows.

One static library target `horcom`, headers included as `horcom/<module>/<name>.hpp`. Modules grow strictly in the phase order of the architecture, each routine lands together with its tests.

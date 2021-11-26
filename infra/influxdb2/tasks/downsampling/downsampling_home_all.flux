// Query to dowsample data from "infra" bucket
// downsample ALL data (no start time) from fromBucket
// intended to be run on bucket with existing history, before 
// we get a periodic downsampling task set up

_every=1h
fromBucket = "home"
toBucket = "home_" + string(v: _every)
	all_data = from(bucket: fromBucket)
		|> range(start: -30d, stop: -25d)
		|> filter(fn: (r) =>
			(r._measurement == "Owens")
				and exists r._value and (r._value >= 0 or r._value < 0))
	all_data
		|> aggregateWindow(every: _every, fn: mean)
		|> set(key: "aggregate", value: "mean")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket)
	all_data
		|> aggregateWindow(every: _every, fn: min)
		|> set(key: "aggregate", value: "min")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket)
	all_data
		|> aggregateWindow(every: _every, fn: max)
		|> set(key: "aggregate", value: "max")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket)
	all_data
		|> aggregateWindow(every: _every, fn: sum)
		|> set(key: "aggregate", value: "sum")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket)
	all_data
		|> map(fn: (r) =>
			({r with _value: float(v: r._value)}))
		|> quantile(
			column: "_value",
			q: 0.999,
			method: "estimate_tdigest",
			compression: 1000.0,
		)
		|> set(key: "aggregate", value: "p99.9")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket, timeColumn: "_stop")
	all_data
		|> map(fn: (r) =>
			({r with _value: float(v: r._value)}))
		|> quantile(
			column: "_value",
			q: 0.99,
			method: "estimate_tdigest",
			compression: 1000.0,
		)
		|> set(key: "aggregate", value: "p99")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket, timeColumn: "_stop")
	all_data
		|> map(fn: (r) =>
			({r with _value: float(v: r._value)}))
		|> quantile(
			column: "_value",
			q: 0.95,
			method: "estimate_tdigest",
			compression: 1000.0,
		)
		|> set(key: "aggregate", value: "p95")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket, timeColumn: "_stop")
	all_data
		|> map(fn: (r) =>
			({r with _value: float(v: r._value)}))
		|> quantile(
			column: "_value",
			q: 0.9,
			method: "estimate_tdigest",
			compression: 1000.0,
		)
		|> set(key: "aggregate", value: "p90")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket, timeColumn: "_stop")
	all_data
		|> aggregateWindow(every: _every, fn: count)
		|> map(fn: (r) =>
			({r with _value: float(v: r._value)}))
		|> set(key: "aggregate", value: "count")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket)
	all_data
		|> aggregateWindow(every: _every, fn: last)
		|> set(key: "aggregate", value: "last")
		|> set(key: "rollup_interval", value: string(v: _every))
		|> to(bucket: toBucket)
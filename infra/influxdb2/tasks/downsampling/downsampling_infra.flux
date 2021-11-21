// Query to dowsample data from "infra" bucket
// https://github.com/influxdata/community-templates/blob/master/downsampling/all_inputs/downsampling_tasks.yml

fromBucket = "infra"
toBucket = "infra_" + string(v: task.every)
	all_data = from(bucket: fromBucket)
		|> range(start: -task.every)
		|> filter(fn: (r) =>
			(r._measurement =~ /^internal_.+$/))
	all_data
		|> aggregateWindow(every: task.every, fn: mean)
		|> set(key: "aggregate", value: "mean")
		|> set(key: "rollup_interval", value: string(v: task.every))
		|> to(bucket: toBucket)
	all_data
		|> aggregateWindow(every: task.every, fn: min)
		|> set(key: "aggregate", value: "min")
		|> set(key: "rollup_interval", value: string(v: task.every))
		|> to(bucket: toBucket)
	all_data
		|> aggregateWindow(every: task.every, fn: max)
		|> set(key: "aggregate", value: "max")
		|> set(key: "rollup_interval", value: string(v: task.every))
		|> to(bucket: toBucket)
	all_data
		|> aggregateWindow(every: task.every, fn: sum)
		|> set(key: "aggregate", value: "sum")
		|> set(key: "rollup_interval", value: string(v: task.every))
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
		|> set(key: "rollup_interval", value: string(v: task.every))
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
		|> set(key: "rollup_interval", value: string(v: task.every))
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
		|> set(key: "rollup_interval", value: string(v: task.every))
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
		|> set(key: "rollup_interval", value: string(v: task.every))
		|> to(bucket: toBucket, timeColumn: "_stop")
	all_data
		|> aggregateWindow(every: task.every, fn: count)
		|> map(fn: (r) =>
			({r with _value: float(v: r._value)}))
		|> set(key: "aggregate", value: "count")
		|> set(key: "rollup_interval", value: string(v: task.every))
		|> to(bucket: toBucket)
	all_data
		|> aggregateWindow(every: task.every, fn: last)
		|> set(key: "aggregate", value: "last")
		|> set(key: "rollup_interval", value: string(v: task.every))
		|> to(bucket: toBucket)
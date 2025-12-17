

# drn hud
- A parcer and runtime for drn notation tasks
- **YOU** are the execution engine, this just helps guide you


```yaml
# dishes.drn
TASK: Fill Dishwasher with dirty items from sink.
CONDITION: Dishwasher is 75% full or greater.
  TASK: Fill dishwasher with dish soap and start load.
```



## TODO:
- EXECUTE: Anything todo with them
  - not really sure if this should be treated like a #include macro, where the lexer eats the EXECUTE directive, and just leves a node behind to show that we are loading perminit or in scope
- TASK: this directive is currently being treated as something that we do one at a time, in the RFC, it is shown as something we stack many up on, and then execute as able, thats something we would like to impl, but will come after we get a basic version working first, the task stacks and executing them should come pretty easy once there is basic execution flow and the major tools are impld


# Credits
- [DRN](https://errantspark.com/drn/) by "Errant Spark"
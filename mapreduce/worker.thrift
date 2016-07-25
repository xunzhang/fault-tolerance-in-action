struct ShutdownReply {
  1:i32 Njobs,
  2:bool OK
}

struct DoJobArgs {
  1:string File,
  2:string Operation,
  3:i32 JobNumber, // this job's number
  4:i32 NumOtherPhase // total number of jobs in other phase (map or reduce
}

struct DoJobReply {
  1:bool OK
}

service WorkerService {
  bool Shutdown(1:ShutdownReply res),
  bool DoJob(1:DoJobArgs arg, 2:DoJobReply res)
}

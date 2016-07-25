struct ShutdownReply {
  1:i32 Njobs,
  2:bool OK
}

service MapReduceService {
  bool Register(1:string workerAddr),
  //bool ShutDown(1:ShutdownReply res)
}

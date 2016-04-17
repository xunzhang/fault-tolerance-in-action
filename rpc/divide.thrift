#
# Toy rpc example: implement divide in server end
#

enum Errors {
  SUCCESS = 0,
  INVALID_REQUEST = 1
}

struct Response {
  1:double result,
  2:Errors error
}

service divide {
  Response div(1:double numerator, 2:double denominator),
  string sayhi()
}

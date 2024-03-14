I have a mixture now of virtual inheritance and CRTP.

Basically, one super simple call root class:

// First a macro I use later
#define GRPC_NATIVE_NAME_REQUEST( name )              \
	void native_name_request() {                                           \
		m_service->Request ## name ## (&m_ctx, &m_request, &m_responder, m_cq, m_cq, this);    \
	}


class RpcCallBase {

    public:
        RpcCallBase() {};
        virtual ~RpcCallBase() {};

        virtual void proceed() noexcept {
              MOOSE_ASSERT_MSG(true, "RPC implementation does not overload proceed()");
        };
};

This is what I use to cast the void ptr to in order to get it to RTTI the right type. Then, on top of this, the actual CRTP base. Like this:

// First a macro I use later
#define GRPC_NATIVE_NAME_REQUEST( name )              \
	void native_name_request() {                                           \
		m_service->Request ## name ## (&m_ctx, &m_request, &m_responder, m_cq, m_cq, this);    \
	}


template< class DerivedType, class RequestType, class ResponseType >
class MyServiceCall : public RpcCallBase {

    public:

        typedef MyServiceCall<DerivedType, RequestType, ResponseType> base_type;

	MyServiceCall() {
             // constructor with service specific stuff such as parent object and so on
             proceed();  like in the example
        }

        void proceed() noexcept override {
             // Much like the example, except:
             if (m_status == CREATE) {
                   m_status = PROCESS;
                   static_cast<DerivedType *>(this)->native_name_request();

             // this is what the macro injects in order to fake the right type in here. See below.
             } else if (m_status == PROCESS) {
		
                    // new object of CRTP	derived type
                    new base_type(m_service, m_cq, m_parent);
                   
                    // CRTP to the actual work, overloaded by derived class
                    grpc::Status retstat = static_cast<DerivedType *>(this)->work();

              }
    // rest of the stuff pretty much like the example except template types
}

and then, each call can be implemented nicely:

class FooMethodCall : public MyServiceCall<FooMethodCall, FooMethodRequest, FooMethodResponse> {

    public:
        GRPC_NATIVE_NAME_REQUEST( FooMethod )   // I know, not perfect but it does the trick

        FooMethodCall(MyService::AsyncService *n_service, ServerCompletionQueue *n_cq, MyServiceImpl *n_parent)
              : base_type(n_service, n_cq, n_parent) {
        }

        grpc::Status work() {
                      
                    // do the actual work and return a status object
        }     
}


//Finally, in the async loop it looks like this:

void MyServiceImpl::HandleRpcs() {

    // Spawn a new CallData instance to serve new clients.
    // The q takes ownership
    new FooMethodCall(&m_service_instance, m_cq.get(), this);
    new BarMethodCall(&m_service_instance, m_cq.get(), this);

    void* tag;  // uniquely identifies a request.

    bool ok;
    while (true) {
        
         if (!m_cq->Next(&tag, &ok)) {
               BOOST_LOG_SEV(logger(), normal) << "service shutting down";
               break;
         }

         RpcCallBase *call = static_cast<RpcCallBase *>(tag);

         if (!ok) {
               // This seems to be the case while the q is draining of events 
               // during shutdown I'm gonna delete them
               delete call;
               continue;
          }

          // hand over to the call object to do the rest
          call->proceed();
     }
}

//And it works. This way I don't need any further type guessing or casting beyond the RpcCallBase 
------------------------------



By imposing these restrictions atop the gRPC++ library, we were able to
simplify implementation of async services [8]:

    class GreeterServiceImpl final : public Greeter::Service
    {
    public:
        using Greeter::Service::Service;

    private:
        void SayHello(bond::ext::grpc::unary_call<HelloRequest, HelloReply> call) override
        {
            HelloRequest request = call.request().Deserialize();

            HelloReply reply;
            reply.message = "hello " + request.name;

            call.Finish(reply);
        }
    };


Wow, that's pretty neat. It looks almost as tidy as the Sync Service did. Thanks for posting.
Only thing I would have with it is, the fact that the async approach forced me to take the route with the one class per call approach was one of the few things I liked about it. With a service of, say, 50 methods a class containing all the impls can grow tremendously. Even if each call is very much separated from the others. Few years back we had some static analysis run over the code that showed glowing red complexity dots over those files. Probably due to size, because they weren't this complex. I hope this gets better with the one-class-per-call way.

Cheers,
Stephan

PS: Amazed Microsoft uses gRPC. And open sources the results. The world we live in.


-------------------------

A more flexible approach would be to have a different CallData-like class for each RPC. However when you get a tag from cq_->Next(), you know that it is a pointer to an object of one of these classes, but you don't know its exact type. To overcome this, you can have them all inherit from a class with a virtual Proceed() member function, implement it as needed in each subclass, and when you get a tag, cast it as CallData and call Proceed().

class CallData {
 public:
  virtual void Proceed() = 0;
};

class HelloCallData final : public CallData {...};
class ByeCallData final : public CallData {...};

...
new HelloCallData(...);
new ByeCallData(...);
cq_->Next(&tag, &ok);
static_cast<CallData*>(tag)->Proceed();
...
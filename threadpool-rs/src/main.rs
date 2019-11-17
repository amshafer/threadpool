// A simple threadpool written in Rust
//
// Austin Shafer - 2019

use std::{thread,time,ptr};
mod threadpool_bindings;
use threadpool_bindings::*;

#[derive(Debug)]
struct ThreadPool {
    c_pool: *mut pool_tag,
}

struct ThreadArg<'a, T> {
    func: Box<dyn Fn(Option<&'a mut T>)>,
    arg: Option<T>,
}

impl ThreadPool {

    unsafe extern "C" fn thread_fn<T>(arg1: *mut ::std::os::raw::c_void) {
        // cast the void ptr to a raw pointer to an int
        let ta_ptr = arg1 as *mut ThreadArg<T>;
        // dereference the raw pointer, and mutably borrow its contents
        let ta_ref = &mut *ta_ptr;
        
        println!("Hello from thread!");
        
        // to call the function stored in `func`, surround the field access with parentheses
        (ta_ref.func)(ta_ref.arg.as_mut());
                
        thread::sleep(time::Duration::from_millis(4));
    }

    pub fn pool_init(thread_count: i32) -> Option<ThreadPool> {
        unsafe {
            let pool = threadpool_bindings::pool_init(thread_count);
            Some(ThreadPool{ c_pool: pool })
        }
    }

    pub fn destroy(&mut self) -> bool {
        unsafe {
            if !self.c_pool.is_null() {
                threadpool_bindings::pool_destroy(self.c_pool);
                return true;
            }
            return false;
        }
    }

    pub fn exec<T>(&mut self, func: Box<dyn Fn(Option<&mut T>)>, arg: Option<T>) {
        unsafe {
            let mut thread_arg = Box::new(ThreadArg {
                func: func,
                arg: arg,
            });
            
            pool_exec(self.c_pool, Some(ThreadPool::thread_fn::<T>),
                      &mut thread_arg as *mut _ as *mut ::std::ffi::c_void);
        }
    }
}

fn worker(arg: Option<&mut i32>) {
    match arg {
        Some(i) => println!("Hello from thread {}!", i),
        None => println!("Hello from a unidentified flying thread!"),
    };
}

fn main() {

    let mut pool = ThreadPool::pool_init(4).unwrap();
    let mut thread_arg: i32 = 16;

    for i in 0..1 {
        let fp: fn(Option<&mut i32>) = worker;
        let bfp = Box::new(fp);
        pool.exec(bfp, None);
    }
    
    pool.destroy();
}

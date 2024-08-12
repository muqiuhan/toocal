pub mod page_size {
    extern crate std;
    use std::sync::Once;

    #[cfg(unix)]
    extern crate libc;

    #[cfg(windows)]
    extern crate winapi;

    /// This function retrieves the system's memory page size.
    ///
    /// # Example
    ///
    /// ```rust
    /// extern crate page_size;
    /// println!("{}", page_size::get());
    /// ```
    pub fn get() -> usize {
        get_helper()
    }

    #[cfg(all(unix))]
    #[inline]
    fn get_helper() -> usize {
        static INIT: Once = Once::new();
        static mut PAGE_SIZE: usize = 0;

        unsafe {
            INIT.call_once(|| PAGE_SIZE = unix::get());
            PAGE_SIZE
        }
    }

    #[cfg(unix)]
    mod unix {
        use libc::{sysconf, _SC_PAGESIZE};

        #[inline]
        pub fn get() -> usize {
            unsafe { sysconf(_SC_PAGESIZE) as usize }
        }
    }

    // WebAssembly section

    // WebAssembly does not have a specific allocation granularity.
    // The page size works well.
    #[cfg(all(
        not(target_os = "emscripten"),
        any(target_arch = "wasm32", target_arch = "wasm64")
    ))]
    #[inline]
    fn get() -> usize {
        // <https://webassembly.github.io/spec/core/exec/runtime.html#page-size>
        65536
    }

    // Windows Section

    #[cfg(all(windows))]
    #[inline]
    fn get_helper() -> usize {
        static INIT: Once = Once::new();
        static mut PAGE_SIZE: usize = 0;

        unsafe {
            INIT.call_once(|| PAGE_SIZE = windows::get());
            PAGE_SIZE
        }
    }

    #[cfg(windows)]
    mod windows {
        #[cfg(feature = "no_std")]
        use core::mem;
        #[cfg(not(feature = "no_std"))]
        use std::mem;

        use winapi::um::sysinfoapi::GetSystemInfo;
        use winapi::um::sysinfoapi::{LPSYSTEM_INFO, SYSTEM_INFO};

        #[inline]
        pub fn get() -> usize {
            unsafe {
                let mut info: SYSTEM_INFO = mem::zeroed();
                GetSystemInfo(&mut info as LPSYSTEM_INFO);

                info.dwPageSize as usize
            }
        }
    }

    // Stub Section

    #[cfg(not(any(unix, windows)))]
    #[inline]
    fn get_helper() -> usize {
        4096 // 4k is the default on many systems
    }

    #[cfg(test)]
    mod tests {
        use super::*;

        #[test]
        fn test_get() {
            #[allow(unused_variables)]
            let page_size = get();
        }
    }
}

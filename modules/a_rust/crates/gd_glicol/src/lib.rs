use glicol::Engine;
use std::collections::VecDeque;

pub struct Glicol {
    pub engine: Engine<128>,
    pub buffer: VecDeque<f32>,
    pub samples: Vec<Vec<f32>>,
}

pub fn glicol_create() -> Box<Glicol> {
    Box::new(Glicol {
        engine: Engine::new(),
        buffer: VecDeque::with_capacity(128),
        samples: Vec::new(),
    })
}

impl Glicol {
    pub fn process(&mut self, size: usize, o_bytes: *mut f32) {
        while self.buffer.len() <= size {
            let (engine_out, _) = self.engine.next_block(vec![]);
            for o in engine_out[0].iter() {
                self.buffer.push_back(*o);
            }
        }
        for i in 0..size {
            unsafe { *o_bytes.offset(i as isize) = self.buffer.pop_front().unwrap() };
        }
    }

    pub fn add_sample(&mut self, name_str: &str, sample: Vec<f32>, channels: usize, sr: usize) {
        self.samples.push(sample);
        let p = self.samples.last().unwrap();
        let p_slice = unsafe { std::slice::from_raw_parts(p.as_ptr(), p.len()) };
        self.engine.add_sample(name_str, p_slice, channels, sr);
    }

    pub fn update_code(&mut self, code: &str) {
        self.engine.update_with_code(code);
    }

    pub fn send_msg(&mut self, msg: &str) {
        self.engine.send_msg(msg);
    }

    pub fn live_coding_mode(&mut self, io: bool) {
        self.engine.livecoding = io;
    }

    pub fn set_bpm(&mut self, bpm: f32) {
        self.engine.set_bpm(bpm);
    }

    pub fn set_track_amp(&mut self, amp: f32) {
        self.engine.set_track_amp(amp);
    }

    pub fn set_sr(&mut self, sr: f32) {
        self.engine.set_sr(sr as usize);
    }

    pub fn set_seed(&mut self, seed: f32) {
        self.engine.set_seed(seed as usize);
    }

    pub fn reset(&mut self) {
        self.engine.reset();
    }
}

#[cxx::bridge(namespace = "glicol")]
pub mod ffi {
    extern "Rust" {
        type Glicol;
        fn glicol_create() -> Box<Glicol>;
        unsafe fn process(&mut self, size: usize, o_bytes: *mut f32);
        fn add_sample(&mut self, name_str: &str, sample: Vec<f32>, channels: usize, sr: usize);
        fn update_code(&mut self, code: &str);
        fn send_msg(&mut self, msg: &str);
        fn live_coding_mode(&mut self, io: bool);
        fn set_bpm(&mut self, bpm: f32);
        fn set_track_amp(&mut self, amp: f32);
        fn set_sr(&mut self, sr: f32);
        fn set_seed(&mut self, seed: f32);
        fn reset(&mut self);
    }
}

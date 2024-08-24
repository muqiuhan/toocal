pub struct Options {
    pub page_size: usize,
    pub fill_percent: FillPercent,
}

pub struct FillPercent {
    pub min_fill_percent: f32,
    pub max_fill_percent: f32,
}

pub const DEFAULT_FILL_PERCENT: FillPercent = FillPercent {
    min_fill_percent: 0.5,
    max_fill_percent: 0.95,
};

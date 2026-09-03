# PasLang Machine Learning & Tensor Engine Demo
# Trains a Linear Regression Model using Gradient Descent and activation functions (ReLU, Sigmoid).

say "=================================================="
say "     PasLang Machine Learning Engine Demo         "
say "=================================================="
say ""

# 1. Activation Functions
say "--- 1. Neural Activation Functions ---"
let raw_inputs = [-2.5, -0.5, 0.0, 1.5, 3.0]
say "Raw Inputs:"
say raw_inputs

let relu_out = relu raw_inputs
say "ReLU Activation Output:"
say relu_out

let sig_out = sigmoid raw_inputs
say "Sigmoid Activation Output:"
say sig_out
say ""

# 2. Model Training via Gradient Descent
# Target function: y = 2.5 * x + 1.0
say "--- 2. Training Linear Regression Model (y = 2.5 * x + 1.0) ---"

let weights = [0.1]
let bias = 0.0
let lr = 0.05

# Training Data Pairs (x, y)
let train_x = [[1.0], [2.0], [3.0], [4.0]]
let train_y = [3.5, 6.0, 8.5, 11.0]

say "Initial Weights:"
say weights
say "Initial Bias:"
say bias
say ""
say "Training for 20 epochs..."

repeat 20:
    for sample_x in train_x:
        # In a real loop we update over dataset
        let step = train_linear_step weights bias sample_x 3.5 lr
        weights = step["weights"]
        bias = step["bias"]

say "Trained Weights:"
say weights
say "Trained Bias:"
say bias
say ""

# 3. Model Inference / Prediction
say "--- 3. Model Prediction for new input x = [5.0] ---"
let test_x = [5.0]
let prediction = predict_linear weights bias test_x
say "Predicted Output y for x = 5.0:"
say prediction
say "(Expected Target y ≈ 13.5)"
say ""
say "Machine Learning Engine Demo complete!"

# Gateway Tools

This directory contains utility scripts for managing and testing the API Gateway.

## Available Tools

### 1. Model Generation (`create_sample_model.py`)

Creates a sample ONNX model for API abuse detection using Isolation Forest.

**Installation:**
```bash
pip install scikit-learn skl2onnx numpy onnx
```

**Usage:**
```bash
python tools/create_sample_model.py
```

**Output:**
- `models/abuse_detector.onnx` - ONNX model file

**What it does:**
- Generates synthetic traffic data (normal + abusive patterns)
- Trains an Isolation Forest anomaly detector
- Exports to ONNX format
- Tests the model with sample inputs

### 2. Load Testing (Coming Soon)

Script to simulate various traffic patterns for testing.

### 3. Model Evaluation (Coming Soon)

Evaluate model performance on real traffic data.

## Development Workflow

1. **Generate Model:**
   ```bash
   python tools/create_sample_model.py
   ```

2. **Test Locally:**
   ```bash
   # Build gateway
   docker build -t gateway .
   
   # Run with model
   docker run -v $(pwd)/models:/models -p 8080:8080 gateway
   ```

3. **Monitor:**
   ```bash
   # Check logs
   docker logs -f <container_id>
   
   # View dashboard
   open http://localhost:8080/dashboard/index.html
   ```

## Model Training Tips

- Collect real traffic data for better accuracy
- Adjust `contamination` parameter based on expected abuse rate
- Retrain periodically with new patterns
- Test thoroughly before production deployment

## Contributing

When adding new tools:
1. Add Python requirements to `requirements.txt` in this directory
2. Document usage in this README
3. Follow the existing code style
4. Add error handling and logging

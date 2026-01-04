#!/usr/bin/env python3
"""
Generate a sample ONNX model for API abuse detection.

This script creates a simple Isolation Forest model trained on synthetic
traffic data and exports it to ONNX format for use with the gateway.

Requirements:
    pip install scikit-learn skl2onnx numpy onnx

Usage:
    python create_sample_model.py
    
Output:
    models/abuse_detector.onnx
"""

import numpy as np
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
import os

try:
    from skl2onnx import convert_sklearn
    from skl2onnx.common.data_types import FloatTensorType
except ImportError:
    print("Error: Required packages not installed.")
    print("Install with: pip install scikit-learn skl2onnx numpy onnx")
    exit(1)


def generate_training_data(n_samples=5000):
    """
    Generate synthetic training data representing normal and abusive traffic patterns.
    
    Features:
    - feature[0]: RPS (requests per second)
    - feature[1]: Burstiness (variance in timing)
    
    Returns:
        np.array: Training data with shape (n_samples, 2)
    """
    print("Generating synthetic training data...")
    
    # Normal traffic patterns (80% of data)
    normal_samples = int(n_samples * 0.8)
    normal_rps = np.random.gamma(shape=2.0, scale=2.0, size=normal_samples)  # Low RPS
    normal_burst = np.random.gamma(shape=3.0, scale=200.0, size=normal_samples)  # Moderate variance
    
    # Abusive traffic patterns (20% of data)
    abuse_samples = n_samples - normal_samples
    
    # Type 1: High-speed bots (high RPS, low burstiness)
    bot_count = abuse_samples // 3
    bot_rps = np.random.gamma(shape=10.0, scale=8.0, size=bot_count) + 50
    bot_burst = np.random.gamma(shape=1.5, scale=50.0, size=bot_count)
    
    # Type 2: Burst attackers (very high burstiness)
    burst_count = abuse_samples // 3
    burst_rps = np.random.gamma(shape=5.0, scale=5.0, size=burst_count) + 20
    burst_burst = np.random.gamma(shape=2.0, scale=1500.0, size=burst_count) + 2000
    
    # Type 3: DDoS (extremely high RPS, high burstiness)
    ddos_count = abuse_samples - bot_count - burst_count
    ddos_rps = np.random.gamma(shape=15.0, scale=10.0, size=ddos_count) + 80
    ddos_burst = np.random.gamma(shape=3.0, scale=1000.0, size=ddos_count) + 1000
    
    # Combine all data
    rps = np.concatenate([normal_rps, bot_rps, burst_rps, ddos_rps])
    burstiness = np.concatenate([normal_burst, bot_burst, burst_burst, ddos_burst])
    
    X = np.column_stack([rps, burstiness])
    
    print(f"  Normal traffic samples: {normal_samples}")
    print(f"  Abusive traffic samples: {abuse_samples}")
    print(f"    - Bot patterns: {bot_count}")
    print(f"    - Burst patterns: {burst_count}")
    print(f"    - DDoS patterns: {ddos_count}")
    print(f"  Total samples: {len(X)}")
    print(f"  Feature ranges:")
    print(f"    - RPS: [{X[:, 0].min():.2f}, {X[:, 0].max():.2f}]")
    print(f"    - Burstiness: [{X[:, 1].min():.2f}, {X[:, 1].max():.2f}]")
    
    return X


def train_model(X_train):
    """
    Train an Isolation Forest anomaly detection model.
    
    Args:
        X_train: Training data
        
    Returns:
        Trained model
    """
    print("\nTraining Isolation Forest model...")
    
    # Configure model
    # contamination=0.15 means we expect ~15% of traffic to be anomalous
    model = IsolationForest(
        contamination=0.15,
        max_samples='auto',
        random_state=42,
        n_estimators=100,
        max_features=2,
        bootstrap=False,
        n_jobs=-1,
        verbose=0
    )
    
    # Train
    model.fit(X_train)
    
    # Test on training data to show distribution
    scores = model.score_samples(X_train)
    predictions = model.predict(X_train)
    
    anomaly_count = np.sum(predictions == -1)
    normal_count = np.sum(predictions == 1)
    
    print(f"  Model trained with {len(X_train)} samples")
    print(f"  Detected anomalies: {anomaly_count} ({100*anomaly_count/len(X_train):.1f}%)")
    print(f"  Normal samples: {normal_count} ({100*normal_count/len(X_train):.1f}%)")
    print(f"  Score range: [{scores.min():.3f}, {scores.max():.3f}]")
    
    return model


def convert_to_onnx(model, output_path):
    """
    Convert sklearn model to ONNX format.
    
    Args:
        model: Trained sklearn model
        output_path: Path to save ONNX model
    """
    print("\nConverting model to ONNX format...")
    
    # Define input type: 2D float tensor with shape [batch_size, 2]
    initial_type = [('float_input', FloatTensorType([None, 2]))]
    
    # Convert
    try:
        onnx_model = convert_sklearn(
            model,
            initial_types=initial_type,
            target_opset=12,
            options={id(model): {'zipmap': False}}  # Simplified output
        )
        
        # Save
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, "wb") as f:
            f.write(onnx_model.SerializeToString())
        
        file_size = os.path.getsize(output_path)
        print(f"  Model saved to: {output_path}")
        print(f"  File size: {file_size / 1024:.2f} KB")
        
    except Exception as e:
        print(f"  Error during conversion: {e}")
        raise


def test_model(model, X_train):
    """
    Test model with example inputs.
    
    Args:
        model: Trained model
        X_train: Training data for reference
    """
    print("\nTesting model with sample inputs...")
    
    test_cases = [
        ("Normal user (low RPS)", [3.0, 250.0]),
        ("Moderate user", [8.0, 500.0]),
        ("Suspicious (high RPS)", [45.0, 800.0]),
        ("Bot-like (very high RPS, low burst)", [85.0, 150.0]),
        ("Burst attack (high burst)", [30.0, 3500.0]),
        ("DDoS (extreme values)", [120.0, 2800.0]),
    ]
    
    print("\n  Risk Assessment:")
    print("  " + "-" * 60)
    
    for name, features in test_cases:
        X_test = np.array([features])
        
        # Get anomaly score (more negative = more anomalous)
        score = model.score_samples(X_test)[0]
        prediction = model.predict(X_test)[0]
        
        # Convert to 0-1 risk score (normalize based on training data range)
        min_score = model.score_samples(X_train).min()
        max_score = model.score_samples(X_train).max()
        risk_score = 1.0 - (score - min_score) / (max_score - min_score)
        risk_score = np.clip(risk_score, 0.0, 1.0)
        
        status = "BLOCK" if risk_score > 0.8 else "WARN" if risk_score > 0.5 else "ALLOW"
        
        print(f"  {name:35s} | Risk: {risk_score:.3f} | {status:5s}")
        print(f"    Features: RPS={features[0]:.1f}, Burst={features[1]:.1f}")
    
    print("  " + "-" * 60)


def main():
    """Main execution function."""
    print("=" * 70)
    print("ML Model Generator for API Abuse Detection Gateway")
    print("=" * 70)
    
    # Configuration
    output_path = "models/abuse_detector.onnx"
    n_samples = 5000
    
    # Generate data
    X_train = generate_training_data(n_samples)
    
    # Train model
    model = train_model(X_train)
    
    # Test model
    test_model(model, X_train)
    
    # Convert to ONNX
    convert_to_onnx(model, output_path)
    
    print("\n" + "=" * 70)
    print("✓ Model generation complete!")
    print("=" * 70)
    print(f"\nNext steps:")
    print(f"  1. Copy '{output_path}' to your gateway container")
    print(f"  2. Update config.json with model path")
    print(f"  3. Rebuild and restart the gateway")
    print(f"  4. Monitor logs for ML inference messages")
    print("\nFor full ONNX Runtime integration, see docs/ML_MODEL_GUIDE.md")


if __name__ == "__main__":
    main()

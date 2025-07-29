#!/usr/bin/env python3
"""
AMNEZIAGAZE VST Log Analyzer
Analyzes real-time VST log files to identify issues and patterns
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
import argparse
import os

def load_log_file(log_path):
    """Load and parse the VST log file"""
    try:
        # Read CSV with proper column names
        df = pd.read_csv(log_path, skiprows=1)  # Skip the header comment
        df.columns = ['Timestamp', 'Level', 'Component', 'Parameter', 'Value', 'Additional_Info']
        
        # Convert timestamp to datetime
        df['Timestamp'] = pd.to_datetime(df['Timestamp'], format='%H:%M:%S.%f')
        
        return df
    except Exception as e:
        print(f"Error loading log file: {e}")
        return None

def analyze_clipping(df):
    """Analyze clipping events"""
    clipping_events = df[df['Parameter'] == 'clipping']
    
    if len(clipping_events) > 0:
        print("\n=== CLIPPING ANALYSIS ===")
        print(f"Total clipping events: {len(clipping_events)}")
        
        # Group by component
        clipping_by_component = clipping_events.groupby('Component').size()
        print("\nClipping events by component:")
        for component, count in clipping_by_component.items():
            print(f"  {component}: {count} events")
        
        # Show worst clipping values
        worst_clipping = clipping_events.nlargest(5, 'Value')
        print("\nWorst clipping events:")
        for _, event in worst_clipping.iterrows():
            print(f"  {event['Timestamp'].strftime('%H:%M:%S.%f')[:-3]} - {event['Component']}: {event['Value']:.3f}")
    else:
        print("\n=== CLIPPING ANALYSIS ===")
        print("No clipping events detected!")

def analyze_parameter_changes(df):
    """Analyze parameter changes"""
    param_changes = df[df['Level'] == 'INFO'][df['Component'] == 'Parameter']
    
    if len(param_changes) > 0:
        print("\n=== PARAMETER CHANGES ===")
        print(f"Total parameter changes: {len(param_changes)}")
        
        # Group by parameter name
        changes_by_param = param_changes.groupby('Parameter').size()
        print("\nMost changed parameters:")
        for param, count in changes_by_param.sort_values(ascending=False).head(10).items():
            print(f"  {param}: {count} changes")
    else:
        print("\n=== PARAMETER CHANGES ===")
        print("No parameter changes logged")

def analyze_audio_levels(df):
    """Analyze audio signal levels"""
    audio_samples = df[df['Parameter'].isin(['input', 'output'])]
    
    if len(audio_samples) > 0:
        print("\n=== AUDIO LEVEL ANALYSIS ===")
        
        # Analyze by component
        components = audio_samples['Component'].unique()
        
        for component in components:
            component_data = audio_samples[audio_samples['Component'] == component]
            
            if len(component_data) > 0:
                input_data = component_data[component_data['Parameter'] == 'input']
                output_data = component_data[component_data['Parameter'] == 'output']
                
                print(f"\n{component}:")
                if len(input_data) > 0:
                    print(f"  Input  - RMS: {np.sqrt(np.mean(input_data['Value']**2)):.4f}, Peak: {input_data['Value'].abs().max():.4f}")
                if len(output_data) > 0:
                    print(f"  Output - RMS: {np.sqrt(np.mean(output_data['Value']**2)):.4f}, Peak: {output_data['Value'].abs().max():.4f}")
                
                # Calculate gain change
                if len(input_data) > 0 and len(output_data) > 0:
                    input_rms = np.sqrt(np.mean(input_data['Value']**2))
                    output_rms = np.sqrt(np.mean(output_data['Value']**2))
                    if input_rms > 0:
                        gain_db = 20 * np.log10(output_rms / input_rms)
                        print(f"  Gain: {gain_db:.2f} dB")

def analyze_effect_states(df):
    """Analyze effect bypass states"""
    effect_states = df[df['Parameter'] == 'bypassed']
    
    if len(effect_states) > 0:
        print("\n=== EFFECT STATES ===")
        
        for component in effect_states['Component'].unique():
            component_states = effect_states[effect_states['Component'] == component]
            latest_state = component_states.iloc[-1]
            state_str = "BYPASSED" if latest_state['Value'] > 0.5 else "ACTIVE"
            print(f"  {component}: {state_str}")

def create_visualizations(df, output_dir="vst_analysis_plots"):
    """Create visualization plots"""
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # Plot 1: Parameter changes over time
    param_changes = df[df['Level'] == 'INFO'][df['Component'] == 'Parameter']
    if len(param_changes) > 0:
        plt.figure(figsize=(12, 6))
        for param in param_changes['Parameter'].unique()[:5]:  # Top 5 most changed
            param_data = param_changes[param_changes['Parameter'] == param]
            plt.plot(param_data['Timestamp'], param_data['Value'], label=param, marker='o', markersize=3)
        
        plt.title('Parameter Changes Over Time')
        plt.xlabel('Time')
        plt.ylabel('Parameter Value')
        plt.legend()
        plt.xticks(rotation=45)
        plt.tight_layout()
        plt.savefig(f"{output_dir}/parameter_changes.png", dpi=150)
        plt.close()
    
    # Plot 2: Clipping events timeline
    clipping_events = df[df['Parameter'] == 'clipping']
    if len(clipping_events) > 0:
        plt.figure(figsize=(12, 6))
        components = clipping_events['Component'].unique()
        colors = plt.cm.Set1(np.linspace(0, 1, len(components)))
        
        for i, component in enumerate(components):
            component_clips = clipping_events[clipping_events['Component'] == component]
            plt.scatter(component_clips['Timestamp'], component_clips['Value'], 
                       label=component, color=colors[i], alpha=0.7)
        
        plt.title('Clipping Events Over Time')
        plt.xlabel('Time')
        plt.ylabel('Clipping Level')
        plt.legend()
        plt.xticks(rotation=45)
        plt.tight_layout()
        plt.savefig(f"{output_dir}/clipping_events.png", dpi=150)
        plt.close()
    
    # Plot 3: Audio levels by component
    audio_samples = df[df['Parameter'].isin(['input', 'output'])]
    if len(audio_samples) > 0:
        components = audio_samples['Component'].unique()
        
        fig, axes = plt.subplots(len(components), 1, figsize=(12, 3*len(components)))
        if len(components) == 1:
            axes = [axes]
        
        for i, component in enumerate(components):
            component_data = audio_samples[audio_samples['Component'] == component]
            
            input_data = component_data[component_data['Parameter'] == 'input']
            output_data = component_data[component_data['Parameter'] == 'output']
            
            if len(input_data) > 0:
                axes[i].plot(input_data['Timestamp'], input_data['Value'], 
                           label='Input', alpha=0.7, color='blue')
            if len(output_data) > 0:
                axes[i].plot(output_data['Timestamp'], output_data['Value'], 
                           label='Output', alpha=0.7, color='red')
            
            axes[i].set_title(f'{component} Audio Levels')
            axes[i].set_ylabel('Amplitude')
            axes[i].legend()
            axes[i].grid(True, alpha=0.3)
        
        plt.xlabel('Time')
        plt.tight_layout()
        plt.savefig(f"{output_dir}/audio_levels.png", dpi=150)
        plt.close()
    
    print(f"\nVisualization plots saved to: {output_dir}/")

def main():
    parser = argparse.ArgumentParser(description='Analyze AMNEZIAGAZE VST log files')
    parser.add_argument('log_file', nargs='?', default='C:/temp/amneziagaze_realtime_log.txt',
                       help='Path to the VST log file')
    parser.add_argument('--plots', action='store_true', help='Generate visualization plots')
    parser.add_argument('--output-dir', default='vst_analysis_plots', 
                       help='Directory for output plots')
    
    args = parser.parse_args()
    
    print("AMNEZIAGAZE VST Log Analyzer")
    print("=" * 40)
    
    if not os.path.exists(args.log_file):
        print(f"Error: Log file not found: {args.log_file}")
        print("\nTo generate a log file:")
        print("1. Load the AMNEZIAGAZE plugin in your DAW")
        print("2. Play some audio through it")
        print("3. The log will be created at: C:/temp/amneziagaze_realtime_log.txt")
        return
    
    # Load and analyze the log
    df = load_log_file(args.log_file)
    if df is None:
        return
    
    print(f"Loaded {len(df)} log entries from {args.log_file}")
    print(f"Time range: {df['Timestamp'].min().strftime('%H:%M:%S')} to {df['Timestamp'].max().strftime('%H:%M:%S')}")
    
    # Run analyses
    analyze_clipping(df)
    analyze_parameter_changes(df)
    analyze_audio_levels(df)
    analyze_effect_states(df)
    
    # Generate plots if requested
    if args.plots:
        try:
            create_visualizations(df, args.output_dir)
        except ImportError:
            print("\nNote: matplotlib not available for plotting")
        except Exception as e:
            print(f"\nError creating plots: {e}")
    
    print(f"\nAnalysis complete! Log file: {args.log_file}")

if __name__ == "__main__":
    main()
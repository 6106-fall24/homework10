import matplotlib.pyplot as plt
import pandas as pd
import argparse

def plot(csv_path, x_axis, y_axis, group_by, output_path='plot.png', filter=None):
    df = pd.read_csv(csv_path)
    
    if filter:
        filter_col, filter_val = filter
        df = df[df[filter_col] == filter_val]
        
    plt.figure(figsize=(12, 8))

    # Get all unique combinations of grouping columns
    unique_combinations = df.groupby(list(group_by)).size().reset_index()[list(group_by)]

    for _, row in unique_combinations.iterrows():
        values = row.values
        mask = pd.Series(True, index=df.index)
        for col, val in zip(group_by, values):
            mask &= (df[col] == val)
            
        data = df[mask]
        if len(data) > 0:
            label = ', '.join(f"{col}={val}" for col, val in zip(group_by, values))
            plt.loglog(data[x_axis], data[y_axis], 'o-', label=label)

    plt.grid(True)
    plt.xlabel(x_axis)
    plt.ylabel(y_axis)
    filter_str = f', {filter[0]}={filter[1]}' if filter else ''
    plt.title(f'Memory Access Patterns Performance {filter_str}')
    plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left')
    plt.tight_layout()
    plt.savefig(output_path)
    plt.close()

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Plot memory access patterns performance')
    parser.add_argument('csv_path', type=str, help='Path to the CSV file')
    parser.add_argument('--x_axis', type=str, help='Column name for x-axis', required=True)
    parser.add_argument('--y_axis', type=str, help='Column name for y-axis', required=True)
    parser.add_argument('--group_by', help='Columns to group by, separated by commas', 
                        type=lambda x: tuple(x.split(',')))
    parser.add_argument('--output_path', type=str, help='Path to the output file', default='plot.png')
    # take filter in format "column=value" (e.g. "size=64"), value has to be int
    parser.add_argument('--filter', type=lambda x: (x.split('=')[0], int(x.split('=')[1])) if '=' in x else None,
                        help='Filter in format "column=value" (e.g. "size=64")')
    args = parser.parse_args()
    plot(args.csv_path, args.x_axis, args.y_axis, args.group_by, args.output_path, args.filter)



import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot():
    '''
    Takes our C++ generated output and creates our two plots
    Plots save in ./build/ directory
    '''
    data = pd.read_csv("./build/output.csv")

    # Assuming your DataFrame is named df
    # Group by the 'V' column and calculate the mean and standard deviation for each group
    result_df = data.groupby('V').agg({
        'approx1-ratio': ['mean', 'std', 'count'],
        'approx2-ratio': ['mean', 'std', 'count'],
        'approx1-time': ['mean', 'std', 'count'],
        'approx2-time': ['mean', 'std', 'count'],
        'cnf-sat-time': ['mean', 'std', 'count']
    }).reset_index()

    # Flatten the multi-level column index
    result_df.columns = ['{}_{}'.format(col[0], col[1].lower()) if col[1] != '' else col[0] for col in result_df.columns]

    # Plot the two ratio means and standard deviations versus V
    # Calculate the standard error of the mean (SEM) for each group
    result_df['approx1-ratio_sem'] = result_df['approx1-ratio_std'] / np.sqrt(result_df['approx1-ratio_count'])
    result_df['approx2-ratio_sem'] = result_df['approx2-ratio_std'] / np.sqrt(result_df['approx2-ratio_count'])

    plt.errorbar(result_df['V'], result_df['approx1-ratio_mean'], yerr=result_df['approx1-ratio_sem'], 
                 label='Approx1 Ratio', linestyle='-', marker='o', color='red', capsize=4)
    plt.errorbar(result_df['V'], result_df['approx2-ratio_mean'], yerr=result_df['approx2-ratio_sem'], 
                 label='Approx2 Ratio', linestyle='--', marker='o', color='blue', capsize=4)

    plt.xlabel('V', fontsize = 15)
    plt.ylabel(r'$vc_{i} \ / \ vc_{cnf-sat}$', fontsize = 16)
    plt.legend(loc = 0, frameon=False )
    plt.xticks(range(5, max(result_df['V']) + 1, 5))
    plt.grid(True, alpha = 0.4)
    plt.savefig("./build/vc-ratio.pdf", dpi=1200)
    plt.show()

    # Plot the two time means and standard deviations versus V
    result_df['approx1-time_sem'] = result_df['approx1-time_std'] / np.sqrt(result_df['approx1-time_count'])
    result_df['approx2-time_sem'] = result_df['approx2-time_std'] / np.sqrt(result_df['approx2-time_count'])
    result_df['cnf-sat-time_sem'] = result_df['cnf-sat-time_std'] / np.sqrt(result_df['cnf-sat-time_count'])

    
    plt.errorbar(result_df['V'], result_df['approx1-time_mean'], yerr=result_df['approx1-time_sem'], 
                 label='Approx1 Time', linestyle='-', marker='.', color = 'red', capsize=4)
    plt.errorbar(result_df['V'], result_df['approx2-time_mean'], yerr=result_df['approx2-time_sem'], 
                 label='Approx2 Time', linestyle='--', marker='.', color='blue', capsize=4)
    plt.errorbar(result_df['V'], result_df['cnf-sat-time_mean'], yerr=result_df['cnf-sat-time_sem'], 
                 label='CNF SAT Time', color = 'black', marker='.', linestyle='-.', capsize=4)

    plt.xlabel('V', fontsize = 15)
    plt.ylabel(r'$ log(t_{ms}$)', fontsize=15)
    plt.legend(loc = 0, frameon=False)
    plt.yscale('log')
    plt.xticks(range(5, max(result_df['V']) + 1, 5))
    plt.grid(True, alpha = 0.4)
    plt.savefig("./build/run-time.pdf", dpi=1200)
    plt.show()
    
if __name__ == '__main__':
    plot()

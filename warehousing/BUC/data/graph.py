# importing the required module
import matplotlib.pyplot as plt

# x axis values
x = [6,13,25, 50, 100, 150, 200]
# corresponding y axis values
y = [492.27, 151.77, 83.37, 42.17, 15.70, 14.21]

# plotting the points
plt.plot(x, y)

# naming the x axis
plt.xlabel('Minimum Support')
# naming the y axis
plt.ylabel('Time taken(sec)')

# giving a title to my graph
plt.title('Block Size is 8KB')

# function to show the plot
plt.show()

# x axis values
x = [2, 4, 8, 16, 32]
# corresponding y axis values
y = [11.57, 11.45, 21.75, 12.32, 11.61]

# plotting the points
plt.plot(x, y)

# naming the x axis
plt.xlabel('Minimum Support')
# naming the y axis
plt.ylabel('Time taken(sec)')

# giving a title to my graph
plt.title('Minimum Support is 100')

# function to show the plot
plt.show()

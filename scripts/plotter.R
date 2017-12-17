library(ggplot2)
library(grid)
library(gridExtra)
library(quantreg)
options(width=220)
options(scipen=50)
cls <- function() cat(rep("\n",100))
cls()

sink("output.txt", append=FALSE, split=TRUE)

# Data Purification
outlier <- abs(data-mean(data)) > 2*sd(data)
data <- data[!outlier]

plot_x <- c()
plot_z <- c()
plot_y <- c()
n <- length(data)

# Find samples of size
for (size in  1:length(data)) {

  # Find the samples of size 'size' with different
  # starts.
  accum_mean <- c()
  for (start in 1:n) {
    y <- c()
    if (start + size -1 <= n) {
      y <- data[start:start+size-1]
    } else {
      y <- data[start:n]
      end <- size - (n - start +1)
      y <- append(y,data[1:end])
    }
    accum_mean <- append(accum_mean, mean(y))
  }

  # Find the relative SD for size
  plot_x <- append(plot_x, size)
  plot_y <- append(plot_y, sd(accum_mean)/mean(data)*100)
  plot_z <- append(plot_z, sd(accum_mean)/mean(accum_mean)*100)
}


# Plot the results
plot_frame <- data.frame(SS=plot_x, RSD=plot_y, M=plot_z)
label <- ""

g1 <-  ggplot2::ggplot(data=plot_frame, ggplot2::aes(x=SS, y=RSD)) +
      ggplot2::geom_point() + ggplot2::geom_line(colour="blue") +
      scale_x_continuous(breaks = seq(min(plot_frame$SS), max(plot_frame$SS), by = 5)) +
      scale_y_continuous(breaks = seq(min(plot_frame$RSD), max(plot_frame$RSD), by = 0.25)) +
      labs(title=label) + xlab("Sample Size") + ylab("Relative SD")

print(g1)
sink()

if(FALSE) {
  #grid.arrange(g1)
  #g <- arrangeGrob(g1,g2,g3)
  #ggsave(file="MBasedProfiles.png", g)
  #plot(plot_x, plot_y, type="o")
  # Remove all the variables
  #rm(list=setdiff(ls(), lsf.str()))
g2 <- ggplot2::ggplot(data=plot_frame, ggplot2::aes(x=SS, y=M)) +
      ggplot2::geom_point() + ggplot2::geom_line(colour="red") +
      scale_x_continuous(breaks = seq(min(plot_frame$SS), max(plot_frame$SS), by = 5)) +
      #scale_y_continuous(breaks = seq(min(plot_frame$M), max(plot_frame$M), by = 0.5)) +
      labs(title=label) + xlab("Sample Size") + ylab("Mean")


}

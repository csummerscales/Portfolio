#########################################
# Author:  Charlie Summerscales         #
# Course:  COMP 597A                    #
# Date:    05/07/2015                   #
# Purpose: Seismic Bump Classification  #
#########################################

#Package Installations
install.packages("class")
install.packages("klaR")
install.packages("caret")
install.packages("e1071")
install.packages("pROC")
#Loading libraries (if necessary)
library("e1071")
library("caret")
library("klaR")
library("class")
library("pROC")

#####################################
# Pre-Processing
#####################################
Seismic <- read.csv("SeismicData.csv", fill = TRUE, strip.white = TRUE, sep = ",")
Original <- Seismic
#convert factors to numeric vectors
Seismic$Seismic <- as.numeric(Seismic$Seismic)
Seismic$Seismoacoustic <- as.numeric(Seismic$Seismoacoustic)
Seismic$Shift <- as.numeric(Seismic$Shift)
Seismic$Ghazard <- as.numeric(Seismic$Ghazard)
#Normalize the data
scaled.Seismic <- scale(Seismic)
scaled.Seismic <- as.data.frame(scaled.Seismic)
#Restore original class value after normalization
scaled.Seismic$class <- Seismic$class
Seismic <- scaled.Seismic
#Create an attributes-only data.frame
train <- Seismic[,1:15]
#Store class values in a factor
trueValues <-as.factor(Seismic[,16])

#########################################
# Modeling prior to attribute selection
#########################################
#KNN method for optimal accuracy
ctrl <- trainControl(method="repeatedcv", repeats=10, classProbs=TRUE, summaryFunction=twoClassSummary)
knnFit <- train(train, trueValues, "knn", trControl=ctrl)
knnFit
knnPredict <- predict(knnFit)
confusionMatrix(knnPredict, Seismic$class, positive='1')

#KNN with a neighbor count of 1
results <- knn.cv(train,trueValues,k=1)
confusionMatrix(results,trueValues, positive='1')

#Naive Bayes without Laplacian Smoothing
model = train(train,trueValues,'nb',trControl=trainControl(method='repeatedcv',repeats=10))
bayesPredict <- predict(model)
confusionMatrix(bayesPredict, Seismic$class, positive='1')

#Naive Bayes with Laplacian Smoothing
mod <- naiveBayes(class ~ ., data=Seismic, laplace=1)
pred <- predict(mod, train, type=c("raw"), threshold=0.001, eps=0)
pred <- as.data.frame(pred)
pred$'3' <- pmax(pred[,1], pred[,2])
pred$'class' <- ifelse(pred[,1]==pred[,3], 0,1)
results <- as.factor(pred$class)
confusionMatrix(results,trueValues, positive='1')

############################################
# Relevance Analysis and Feature Selection
############################################
#Linear Squares modeling
fit <- lm(class ~ ., data=Seismic)
summary(fit)

#Attribute Selection using Relevance Analysis
train <- Seismic[,c(4,5,9,10,11,12)]
train2 <- Seismic[,-which(names(Seismic) %in% (names(train)))]
train2 <- train2[,-10]
train2$Seismoacoustic <- as.numeric(train2$Seismoacoustic)
train2$Shift <- as.numeric(train2$Shift)
train2$Ghazard <- as.numeric(train2$Ghazard)

############################################
# Modeling Post Attribute Selection
############################################
#Run KNN with high t-value attributes only
results <- knn.cv(train,trueValues,k=1)
confusionMatrix(results,trueValues, positive='1')
#Run Knn with low t-value attributes only
results <- knn.cv(train2,trueValues,k=1)
confusionMatrix(results,trueValues, positive='1')

#Naive Bayes with high t-value attributes only
mod <- naiveBayes(class ~ ., data=Seismic[,c(4,5,9,10,11,12,16)], laplace=1)
pred <- predict(mod, train, type=c("raw"), threshold=0.001, eps=0)
pred <- as.data.frame(pred)
pred$'3' <- pmax(pred[,1], pred[,2])
pred$'class' <- ifelse(pred[,1]==pred[,3], 0,1)
results <- as.factor(pred$class)
confusionMatrix(results,trueValues, positive='1')
#Naive Bayes with low t-value attributes only
mod <- naiveBayes(class ~ ., data=Seismic[,c(1,2,3,6,7,8,13,14,15,16)], laplace=1)
pred <- predict(mod, train2, type=c("raw"), threshold=0.001, eps=0)
pred <- as.data.frame(pred)
pred$'3' <- pmax(pred[,1], pred[,2])
pred$'class' <- ifelse(pred[,1]==pred[,3], 0,1)
results <- as.factor(pred$class)
levels(results) <- c("0","1")
confusionMatrix(results,trueValues, positive='1')
from __future__ import print_function
from pyspark import SparkConf, SparkContext
from pyspark.ml.feature import CountVectorizer
from pyspark.sql import SQLContext
from pyspark.sql.functions import udf, when 
from pyspark.sql.types import StringType, ArrayType, IntegerType, FloatType, StructType, StructField, BooleanType, TimestampType
import pyspark.sql.functions as F
from pyspark.ml.classification import LogisticRegression
from pyspark.ml.tuning import CrossValidator, ParamGridBuilder, CrossValidatorModel
from pyspark.mllib.evaluation import BinaryClassificationMetrics
from pyspark.ml.evaluation import BinaryClassificationEvaluator
import matplotlib.pyplot as plt
import os
import glob
import shutil
import math

from cleantext import sanitize

"""
Task 1: returns (comments, submissions, labeled_data)
Saves in parquets for optimized reading.

"""


def task1_load_comments_submissions_labeled_data(context):
    # TASK 1
    # Code for task 1...
    print("reading comments...")
    if not os.path.exists('comments-minimal.parquet'):
        print("comments parquet doesn't exist. Creating...")
        comments = context.read.json("comments-minimal.json.bz2")
        comments.write.parquet('comments-minimal.parquet')
    else:
        comments = context.read.parquet("comments-minimal.parquet")

    print("reading submissions...")
    if not os.path.exists('submissions.parquet'):
        print("submissions parquet doesn't exist. Creating...")
        submissions = context.read.json("submissions.json.bz2")
        submissions.write.parquet('submissions.parquet')
    else:
        submissions = context.read.parquet("submissions.parquet")

    print("reading labeled_data...")
    if not os.path.exists('labeled_data.parquet'):
        print("labeled_data parquet doesn't exist. Creating...")
        labeled_schema = StructType([
            StructField("Input_id", StringType()),
            StructField("labeldem", IntegerType()),
            StructField("labelgop", IntegerType()),
            StructField("labeldjt", IntegerType())
        ])
        labeled_data = context.read\
            .format("com.databricks.spark.csv")\
            .schema(labeled_schema)\
            .option("header", "true")\
            .option("mode", "DROPMALFORMED")\
            .load("labeled_data.csv") 
        labeled_data.write.parquet('labeled_data.parquet')
    else:
        labeled_data = context.read.parquet("labeled_data.parquet")

    return comments, submissions, labeled_data


"""
task 2:
    Inner joins comments and labeled_data dataframes on comment id.
    Return a dataframe that contains only these columns:
    'body', 'id' 'subreddit', 'labeldjt'

"""



def task2_prune_dataframes(comments, labeled_data):
    comments_pruned = comments.select('body', 'id', 'subreddit')
    result = comments_pruned.join(labeled_data, comments_pruned.id == labeled_data.Input_id).select('body', 'id','subreddit','labeldjt')
    return result


"""
task 4:
Create function to generate unigrams, bigrams and trigrams for each comment using sanitize()

"""


def task4_generate_grams_udf(context):
    # TASK 4
    # Code for task 4...
    context.udf.register("sanitize", sanitize, ArrayType(StringType()))



""" 
Task 5: combine {uni, bi, tri} grams and participating subreddits into same column
'ngrams'.

"""


def task5_combine_trigrams_subreddits(labeled_comments):
    # TASK 5
    # Code for task 5...
    sanitize_udf = udf(sanitize, ArrayType(StringType()))
    return labeled_comments.select('*', sanitize_udf('body').alias('ngrams'))



"""
Task 6A: create a sparse feature vector for all tokens that appear at least 10 times
create a feature vector on column 'features' for each comment record.
Create a new countVectorizer Model, or use a pre-existing one (for task 9)

Returns updated_df, countVectorizerModel


"""


def task6a_create_feature_vector(labeled_comments, countVecModel=None):
    # TASK 6A
    # Code for task 6A...
    if countVecModel is None:
        cv = CountVectorizer(inputCol="ngrams", outputCol="features", minDF=10.0, binary=True)
        my_countVecModel = cv.fit(labeled_comments)
    else:
        my_countVecModel = countVecModel
    result = my_countVecModel.transform(labeled_comments)
    return result, my_countVecModel



"""
Task 6B:

generate two new columns in labeled_comments: poslabel, neglabel.
poslabel is 1 if positive, 0 if otherwise.
neglabel is 1 if negative, 0 if otherwise.

returns the new dataframe.

"""


def task6b_pos_neg_labels(labeled_comments):
    # TASK 6B
    # Code for task 6B...
    return labeled_comments.select(\
        '*',\
        when(labeled_comments.labeldjt ==1, 1).otherwise(0).alias('poslabel'),\
        when(labeled_comments.labeldjt ==-1, 1).otherwise(0).alias('neglabel')\
    )



"""
Task 7: Train ML model.
Create and fit two binary classifiers: posModel and negModel.
Save to pos.model, neg.model in current cwd.

In current cwd,
if pos.model exists, then skip training for pos. Load this instead.
if neg.model exists, then skip training for neg. Load this instead.

Returns posModel, negModel

"""



def get_model_eval_metrics(labeled_test_comments, model, model_name, label_name):
        transformed_comments = model.transform(labeled_test_comments)
        predictionAndLabels = transformed_comments.rdd.map(\
            lambda lp: (float(lp.probability[1]), float(lp[label_name]))\
        )
        metrics = BinaryClassificationMetrics(predictionAndLabels)
        print("auPR for {}: {}".format(model_name, metrics.areaUnderPR))
        print("auROC for {}: {}".format(model_name, metrics.areaUnderROC))





def task7_train_ml_model(labeled_comments):
    # TASK 7
    # Code for task 7...
    if os.path.exists("pos.model"):
        print("posModel exists. Skipping training...")
        posModel = CrossValidatorModel.load("pos.model") # works well
    else:
        # Initialize positive logistic regression models.
        poslr = LogisticRegression(labelCol="poslabel", featuresCol="features", maxIter=10)
        # This is a binary classifier so we need an evaluator that knows how to with binary classifiers.
        posEvaluator = BinaryClassificationEvaluator(labelCol="poslabel")
        # There are a few parameters associated with logistic regression. We do know what they are a priori.
        # We do a grid search to find the best parameters. We can replace [1.0] list of values to try.
        # We will assume the parameter is 1.0. Grid search takes forever. 
        posParamGrid = ParamGridBuilder().addGrid(poslr.regParam, [1.0]).build() 
        # We initialize a 5 fold cross-validation pipeline.
        posCrossval = CrossValidator(
            estimator=poslr, 
            evaluator=posEvaluator, 
            estimatorParamMaps=posParamGrid, 
            numFolds=5)
        # Although crossvalidation creates its own train/test sets for # tuning, we still need a labeled test set, because it is not # accessible from the crossvalidator (argh!)
        # Split the data 50/50
        pos = labeled_comments.select('features', 'poslabel')
        posTrain, posTest = pos.randomSplit([0.5, 0.5]) 
        print("Training positive classifier...") 
        posModel = posCrossval.fit(posTrain) 
        print("training complete. Saving positive classifier..")
        posModel.save("pos.model")
        print("evaluating with testSet")
        get_model_eval_metrics(posTest, posModel, "positive classifier", "poslabel")


    if os.path.exists("neg.model"):
        print("negModel exists. Skipping training...")
        negModel = CrossValidatorModel.load("neg.model") 
    else:
        neglr = LogisticRegression(labelCol="neglabel", featuresCol="features", maxIter=10)
        # This is a binary classifier so we need an evaluator that knows how to with binary classifiers.

        negEvaluator = BinaryClassificationEvaluator(labelCol="neglabel")

        negParamGrid = ParamGridBuilder().addGrid(neglr.regParam, [1.0]).build()

        negCrossval = CrossValidator( 
            estimator=neglr,
            evaluator=negEvaluator, 
            estimatorParamMaps=negParamGrid, 
            numFolds=5)

        neg = labeled_comments.select('features', 'neglabel')
        negTrain, negTest = neg.randomSplit([0.5, 0.5]) # Train the models
        print("Training negative classifier...")
        negModel = negCrossval.fit(negTrain)
        print("training complete. Saving negative classifier..")
        negModel.save("neg.model")
        print("evaluating with testSet:")
        get_model_eval_metrics(negTest, negModel, "negative classifier", "neglabel")
    return posModel, negModel

"""
Task 8: 
    create a comment information dataframe, adding 3 extra columns:
    1. The timestamp when the comment was created ('created_utc')
    2. The title of the submission (post) that the comment was made on ('title')
    3. The state that the commenter is from. ('state')
"""

def task8_get_comment_info(comments, submissions):
    # TASK 8
    # Code for task 8...
    prefix_trim = udf(lambda x:x[3:],StringType())
    selected_comm = comments.select('id', 'body', 'author_flair_text', prefix_trim('link_id').alias('link_id'), 'created_utc', 'score' )
    selected_comm = selected_comm.alias("selected_comm")
    submissions = submissions.alias("submissions")
    result = selected_comm.join(submissions, selected_comm.link_id == submissions.id)\
            .select(F.col('selected_comm.id').alias('id'),\
            F.col('selected_comm.link_id').alias('submission_id'),\
            F.col('selected_comm.body').alias('body') ,\
            F.col('selected_comm.author_flair_text').alias('state'),\
            F.col('selected_comm.created_utc').alias('created_utc'),\
            F.col('submissions.title').alias('title'),\
            F.col('submissions.score').alias('score'),\
            F.col('selected_comm.score').alias('comm_score'))
    return result




"""
TASK 9:
Using task 8's comments_df, we:
    - remove comments that contain  '/s' or '&gt'
    - generate ngrams (task 5)
    - countVectorize these ngrams (task 9)
    - transform them into task9Results by posModel 
        and negModel logistic classifiers

"""


def task9_evaluate_classifiers(context, task8_comments_df, countVecModel, posModel, negModel, posThreshold= 0.205, negThreshold=0.379):
    # TASK 9
    # Code for task 9...
    if os.path.exists("task9Results.parquet"):
        print("task9Results.parquet exists. Reading the parquet..")
        results = context.read.parquet('task9Results.parquet')
        print("OK")
    else:
        print("executing task 9 transformations with posThreshold={}, negThreshold={} ...".format(posThreshold, negThreshold))
        task9_comments = task8_comments_df.filter(~ task8_comments_df.body.like('%/s%'))
        task9_comments = task8_comments_df.filter(~ task8_comments_df.body.like('&gt%'))
        task9_comments = task5_combine_trigrams_subreddits(task9_comments)
        task9_comments, countVectorizerModel = task6a_create_feature_vector(
            task9_comments, countVecModel=countVecModel)
        prob = udf(lambda v: float(v[1]), FloatType())
        results = posModel.transform(task9_comments).select('*', F.col('probability').alias('pos_probability'))
        results = results.drop('probability').drop('prediction').drop('rawPrediction')
        results = negModel.transform(results)
        results = results.select('*', F.col('prediction').alias('neg_pred'), F.col('probability').alias('neg_probability'))
        results = results.drop('probability').drop('prediction').drop('rawPrediction')
        results = results.select(
            'id', 
            'state', 
            'created_utc', 
            'title', 
            'score',
            'comm_score',
            'submission_id',
            when(prob('pos_probability') > posThreshold, 1).otherwise(0).alias('pos'),\
            when(prob('neg_probability') > negThreshold, 1).otherwise(0).alias('neg'),\
        )
        print("writing task9Results.parquet..")
        results.write.parquet('task9Results.parquet')
        print("done")

    return results

"""
Helper function to save csvs into csvname.
This is needed because the pyspark version creates some directory
and has a part-.....csv file inside it, which contains our desired csv.

"""

def save_csv(df, csvname):
    temp_foldername = "{}.temp".format(csvname)
    try:
        df.repartition(1).write\
            .format("com.databricks.spark.csv")\
            .option("header", "true")\
            .save(temp_foldername)
    except:
        # delete temp.
        shutil.rmtree(temp_foldername)
        return
    partfile = glob.glob(os.path.join(temp_foldername, "part*"))
    if len(partfile) == 0:
        print("SAVE CSV for ", csvname,"ERROR")
    else:
        os.rename(partfile[0], csvname)
        shutil.rmtree(temp_foldername)


def task10_computations(context, task9Results, submissions):
    # TASK 10
    # Code for task 10...

    # Task 10.1
    try:
        task10_1 = context.read.load('task10_1.csv', format= 'csv', sep=',', header= "true")
    except:
        task10_1 = task9Results.select('submission_id','pos', 'neg').groupBy('submission_id')\
            .agg(\
                (F.sum('pos')/F.count(F.lit(1))).alias('posPercent'),\
                (F.sum('neg')/F.count(F.lit(1))).alias('negPercent'),\
            )
        save_csv(task10_1, "task10_1.csv")

    # Task 10.2
    try:
        task10_2 = context.read.load('task10_2.csv', format= 'csv', sep=',', header= "true")
    except:
        task10_2 = task9Results.select('pos', 'neg', F.from_unixtime('created_utc', 'yyyy-MM-dd').alias('date')).groupBy('date')\
                .agg(\
                    (F.sum('pos')/ F.count(F.lit(1))).alias('pos_pct'),\
                    (F.sum('neg')/ F.count(F.lit(1))).alias('neg_pct')\
                )\
                .select('pos_pct', 'neg_pct', 'date') 
        save_csv(task10_2, "task10_2.csv")

    # Task 10.3
    try:
        task10_3 = context.read.load('task10_3.csv', format= 'csv', sep=',', header= "true")
    except:
        states = [state.lower() for state in ['Alabama', 'Alaska', 'Arizona', 'Arkansas', 'California', 'Colorado', 'Connecticut', 'Delaware', 'District of Columbia', 'Florida', 'Georgia', 'Hawaii', 'Idaho', 'Illinois', 'Indiana', 'Iowa', 'Kansas', 'Kentucky', 'Louisiana', 'Maine', 'Maryland', 'Massachusetts', 'Michigan', 'Minnesota', 'Mississippi', 'Missouri', 'Montana', 'Nebraska', 'Nevada', 'New Hampshire', 'New Jersey', 'New Mexico', 'New York', 'North Carolina', 'North Dakota', 'Ohio', 'Oklahoma', 'Oregon', 'Pennsylvania', 'Rhode Island', 'South Carolina', 'South Dakota', 'Tennessee', 'Texas', 'Utah', 'Vermont', 'Virginia', 'Washington', 'West Virginia', 'Wisconsin', 'Wyoming']]
        is_state_udf = udf(lambda v: bool(str(v).lower() in states), BooleanType())
        task10_3 = task9Results.filter(is_state_udf(task9Results.state))\
            .groupBy('state')\
            .agg(\
                (F.sum('pos') / F.count(F.lit(1))).alias('pct_pos'),\
                (F.sum('neg') / F.count(F.lit(1))).alias('pct_neg'),\
            )\
            .select('state', 'pct_pos', 'pct_neg')
        save_csv(task10_3, "task10_3.csv")

    # 10.4 comment score vs pos and neg pct plot
    try:
        task10_4_comments = context.read.load('task10_4_comments.csv', format= 'csv', sep=',', header= "true")
    except:
        task10_4_comments = task9Results\
            .groupBy('comm_score')\
            .agg(\
                (F.sum('pos') / F.count(F.lit(1))).alias('pct_pos'),\
                (F.sum('neg') / F.count(F.lit(1))).alias('pct_neg'),\
            )\
            .select('comm_score', 'pct_pos', 'pct_neg')
        save_csv(task10_4_comments, "task10_4_comments.csv")

    # 10.4 - submission score vs pos pct plot
    try:
        task10_4_submissions = context.read.load('task10_4_submissions.csv', format= 'csv', sep=',', header= "true")
    except:
        task10_4_submissions = task9Results\
            .groupBy('score')\
            .agg(\
                (F.sum('pos') / F.count(F.lit(1))).alias('pct_pos'),\
                (F.sum('neg') / F.count(F.lit(1))).alias('pct_neg'),\
            )\
            .select('score', 'pct_pos', 'pct_neg')
        save_csv(task10_4_submissions, "task10_4_submissions.csv")

    # 10.4 -top 10 pos submission stories
    try:
        task10_4_top10Pos_stories = context.read.load('task10_4_top10Pos_stories.csv', format= 'csv', sep=',', header= "true")
    except:
        task10_4_top10Pos_stories = task9Results.groupBy('title')\
            .agg((F.sum('pos') / F.count(F.lit(1))).alias('pct_pos'), F.count(F.lit(1)).alias('count'))\
            .orderBy(F.desc('pct_pos'), F.desc('count')).limit(10)\
            .select('title', 'pct_pos')
        save_csv(task10_4_top10Pos_stories, "task10_4_top10Pos_stories.csv")

    # 10.4 -top 10 neg submission stories
    try:
        task10_4_top10Neg_stories = context.read.load('task10_4_top10Neg_stories.csv', format= 'csv', sep=',', header= "true")
    except:
        task10_4_top10Neg_stories = task9Results.groupBy('title')\
            .agg((F.sum('neg') / F.count(F.lit(1))).alias('pct_neg'), F.count(F.lit(1)).alias('count'))\
            .orderBy(F.desc('pct_neg'), F.desc('count')).limit(10)\
            .select('title', 'pct_neg')
        save_csv(task10_4_top10Neg_stories, "task10_4_top10Neg_stories.csv")





def main(context):
    """Main function takes a Spark SQL context."""
    # YOUR CODE HERE
    # YOU MAY ADD OTHER FUNCTIONS AS NEEDED  
    
    comments, submissions, labeled_data = task1_load_comments_submissions_labeled_data(context)
    labeled_comments = task2_prune_dataframes(comments, labeled_data)
    task4_generate_grams_udf(context)
    labeled_comments = task5_combine_trigrams_subreddits(labeled_comments)
    labeled_comments, countVectorizerModel = task6a_create_feature_vector(labeled_comments)
    labeled_comments = task6b_pos_neg_labels(labeled_comments)
    posModel, negModel = task7_train_ml_model(labeled_comments)
    task8_comments  = task8_get_comment_info(comments, submissions)
    task9_results = task9_evaluate_classifiers(context, task8_comments, countVectorizerModel, posModel, negModel)
    task10_computations(context, task9_results, submissions)


if __name__ == "__main__":
    conf = SparkConf().setAppName("CS143 Project 2B")
    conf = conf.setMaster("local[*]")
    sc   = SparkContext(conf=conf)
    sqlContext = SQLContext(sc)
    sc.addPyFile("cleantext.py")
    main(sqlContext)

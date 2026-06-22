#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class RandomForest {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        uint8_t votes[2] = { 0 };
                        // tree #1
                        if (x[1] <= 0.09000000357627869) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[0] <= 45.93000030517578) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #2
                        if (x[2] <= 10.0) {
                            if (x[0] <= 21.734999656677246) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[1] <= 25.429999351501465) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #3
                        if (x[2] <= 10.0) {
                            if (x[1] <= 27.364998817443848) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #4
                        if (x[2] <= 10.0) {
                            if (x[0] <= 23.75) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #5
                        if (x[0] <= 25.264999389648438) {
                            if (x[0] <= 10.710000038146973) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[2] <= 10.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #6
                        if (x[2] <= 10.0) {
                            if (x[1] <= 32.25) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #7
                        if (x[1] <= 0.09000000357627869) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[1] <= 28.024998664855957) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #8
                        if (x[1] <= 0.19499999284744263) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[1] <= 27.364998817443848) {
                                if (x[0] <= 10.710000038146973) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[2] <= 10.0) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #9
                        if (x[2] <= 10.0) {
                            if (x[1] <= 28.024998664855957) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #10
                        if (x[2] <= 10.0) {
                            if (x[1] <= 32.554999351501465) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            votes[1] += 1;
                        }

                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 2; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                protected:
                };
            }
        }
    }
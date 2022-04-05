struct runPayload
{
    int runId = 0;
    int elem = 0;
    bool operator<(const runPayload &rp)
    {
        if (runId != rp.runId)
            return runId < rp.runId;
        return elem < rp.elem;
    }
    bool operator<=(const runPayload &rp)
    {
        if (runId != rp.runId)
            return runId < rp.runId;
        return elem <= rp.elem;
    }
};
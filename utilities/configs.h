class Config
{
public:
    static Config &getInstance()
    {
        static Config instance;
        return instance;
    }

    const std::string &getDbName() const { return dbName; }
    void setDbName(const std::string &name) { dbName = name; }
    double getBoletoCosto() const { return 7.5; }
    int getExpiration() const { return 10; }

private:
    Config() : dbName("busup-v2.db") {}
    ~Config() {}

    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    std::string dbName;
};

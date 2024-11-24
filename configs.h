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

private:
    Config() : dbName("boletos.db") {}
    ~Config() {}

    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    std::string dbName;
};

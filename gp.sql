-- phpMyAdmin SQL Dump
-- version 4.8.1
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: 2018-11-07 16:44:04
-- 服务器版本： 5.5.60-log
-- PHP Version: 7.2.6

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `gp`
--

-- --------------------------------------------------------

--
-- 表的结构 `mc`
--

CREATE TABLE `mc` (
  `user` varchar(32) NOT NULL,
  `uuid` varchar(32) NOT NULL,
  `mc` varchar(32) NOT NULL,
  `status` int(11) NOT NULL,
  `pub_ip` varchar(16) DEFAULT NULL,
  `pri_ip` varchar(16) DEFAULT NULL,
  `login_date` datetime DEFAULT NULL,
  `logout_date` datetime DEFAULT NULL,
  `create_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- 表的结构 `user`
--

CREATE TABLE `user` (
  `user` varchar(32) NOT NULL,
  `pwd` varchar(32) NOT NULL,
  `status` int(11) NOT NULL,
  `create_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `remark` varchar(32) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- 转存表中的数据 `user`
--

INSERT INTO `user` (`user`, `pwd`, `status`, `create_date`, `remark`) VALUES
('15011457740', '8zIju2Kt', 1, '2018-11-07 01:30:43', 'test');

-- --------------------------------------------------------

--
-- 表的结构 `uuid`
--

CREATE TABLE `uuid` (
  `user` varchar(32) NOT NULL,
  `uuid` varchar(32) NOT NULL,
  `count` int(11) NOT NULL,
  `expire_date` datetime NOT NULL,
  `create_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `remark` varchar(32) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

--
-- 转存表中的数据 `uuid`
--

INSERT INTO `uuid` (`user`, `uuid`, `count`, `expire_date`, `create_date`, `remark`) VALUES
('15011457740', 'db1ac97cf2bb5bab8481b0614346852f', 1, '2018-11-10 00:00:00', '2018-11-06 16:00:00', NULL);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `mc`
--
ALTER TABLE `mc`
  ADD PRIMARY KEY (`user`,`uuid`,`mc`);

--
-- Indexes for table `user`
--
ALTER TABLE `user`
  ADD PRIMARY KEY (`user`);

--
-- Indexes for table `uuid`
--
ALTER TABLE `uuid`
  ADD PRIMARY KEY (`user`,`uuid`);
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;

--zadanie 1
select o.EmployeeID , year(o.OrderDate) year, month(o.OrderDate) month,count(*) as 'liczba zamowien' from 
Orders o group by o.EmployeeID, year(o.OrderDate), month(o.OrderDate) order by o.EmployeeID, [year] , [month]  

--zadanie 2
select p.CategoryID ,max(p.UnitPrice), min(p.UnitPrice ) from Products p group by p.CategoryID 